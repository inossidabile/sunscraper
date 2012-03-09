require 'socket'

# @private
module Sunscraper
  module Standalone
    @last_query_id = 0

    @rpc_mutex    = Mutex.new
    @rpc_waiters  = {}
    @rpc_results  = {}
    @rpc_thread   = nil

    RPC_LOAD_HTML = 1
    RPC_LOAD_URL  = 2
    RPC_WAIT      = 3
    RPC_FETCH     = 4
    RPC_DISCARD   = 5

    class << self
      attr_reader :rpc_mutex, :rpc_waiters, :rpc_results

      def create
        @rpc_mutex.synchronize do
          @last_query_id += 1
          @last_query_id
        end
      end

      def load_html(query_id, html)
        perform_rpc query_id,
          request:     RPC_LOAD_HTML,
          data:        html
      end

      def load_url(query_id, url)
        perform_rpc query_id,
          request:     RPC_LOAD_URL,
          data:        url
      end

      def wait(query_id, timeout)
        perform_rpc query_id,
          request:     RPC_WAIT,
          data:        [timeout].pack("N"),
          want_result: true
      end

      def fetch(query_id)
        perform_rpc query_id,
          request:     RPC_FETCH,
          want_result: true
      end

      def discard(query_id)
        perform_rpc query_id,
          request:     RPC_DISCARD
      end

      private

      def perform_rpc(query_id, options={})
        data  = options[:data] || ""
        block = options[:want_result]

        @rpc_mutex.synchronize do
          if @rpc_thread.nil?
            @rpc_thread = Standalone::Thread.new(::Thread.current)

            # Some fucko decided not to put any semaphores in Ruby,
            # _and_ restrict Mutexes to be unlocked only from the thread
            # which has locked them.
            #
            # Please, kill yourself if you're reading this.
            ::Thread.stop
          end

          @rpc_thread.perform(query_id, options[:request], data)

          if block
            @rpc_waiters[query_id] = Thread.current
          end
        end

        if block
          Thread.stop
          @rpc_results[query_id]
        end
      ensure
        if block
          @rpc_waiters.delete query_id
          @rpc_results.delete query_id
        end
      end
    end

    class Thread < ::Thread
      def initialize(creator)
        @creator = creator

        super do
          @parent = Sunscraper::Standalone
          work
        end
      end

      def perform(query_id, request, data)
        @socket.write([query_id, request, data.length, data].pack("NNNa*"))
      end

      private

      def work
        if ::Sunscraper.os_x?
          # Fuck you, OS X.
          suffix = ".app/Contents/MacOS/sunscraper"
        else
          suffix = RbConfig::CONFIG["EXEEXT"]
        end

        executable = File.join(Gem.loaded_specs['sunscraper'].full_gem_path,
                          'ext', 'standalone', "sunscraper#{suffix}")

        server_path = "/tmp/sunscraper.#{Process.pid}.sock"
        server = UNIXServer.new(server_path)

        if Kernel.respond_to? :spawn
          pid = Kernel.spawn "#{executable} #{server_path}"
        else
          # rbx does not have Kernel.spawn (yet). Sigh...
          pid = fork { exec executable, server_path }
        end

        Process.detach pid

        @socket = server.accept

        server.close
        FileUtils.rm server_path

        # See above.
        @creator.wakeup

        loop do
          header = @socket.read(4 * 3)
          query_id, request, data_length = header.unpack("NNN")
          data   = @socket.read(data_length) if data_length > 0

          @parent.rpc_mutex.synchronize do
            if !@parent.rpc_waiters.include?(query_id)
              $stderr.puts "Sunscraper/standalone: no waiter for #{query_id}"
            else
              @parent.rpc_results[query_id] = data
              @parent.rpc_waiters[query_id].wakeup
            end
          end
        end
      rescue Exception => e
        $stderr.puts "Sunscraper error: #{e.class}: #{e.message}"
        e.backtrace.each do |line|
          $stderr.puts "  #{line}"
        end
      ensure
        @socket.close
        Process.kill pid
      end
    end
  end
end