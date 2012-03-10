require 'socket'

# @private
module Sunscraper
  module Standalone
    @last_query_id = 0

    @rpc_mutex    = Mutex.new
    @rpc_waiters  = {}
    @rpc_results  = {}
    @rpc_thread   = nil

    RPC_LOAD_URL  = 1
    RPC_LOAD_HTML = 2
    RPC_WAIT      = 3
    RPC_FETCH     = 4
    RPC_FINALIZE  = 5

    class << self
      attr_reader :rpc_mutex, :rpc_waiters, :rpc_results

      def create
        @rpc_mutex.synchronize do
          if @rpc_thread.nil?
            Thread.current[:sunscraper_error] = nil
            @rpc_thread = RPCThread.new(Thread.current)

            # Some fucko decided not to put any semaphores in Ruby,
            # _and_ restrict Mutexes to be unlocked only from the thread
            # which has locked them.
            #
            # Please, kill yourself if you're reading this.
            Thread.stop

            if error = Thread.current[:sunscraper_error]
              @rpc_thread = nil
              raise error
            end
          end

          @last_query_id += 1
          @last_query_id
        end
      end

      def load_url(query_id, url)
        perform_rpc query_id,
          request:     RPC_LOAD_URL,
          data:        url
      end

      def load_html(query_id, html, baseUrl)
        html, baseUrl = [html, baseUrl].map(&:to_s)
        perform_rpc query_id,
          request:     RPC_LOAD_HTML,
          data:        [html.length, html, baseUrl.length, baseUrl].pack("Na*Na*")
      end

      def wait(query_id, timeout)
        result = perform_rpc query_id,
          request:     RPC_WAIT,
          data:        [timeout].pack("N"),
          want_result: true
        code, = result.unpack("N")

        code == 1 # true
      end

      def fetch(query_id)
        perform_rpc query_id,
          request:     RPC_FETCH,
          want_result: true
      end

      def finalize(query_id)
        perform_rpc query_id,
          request:     RPC_FINALIZE
      end

      private

      def perform_rpc(query_id, options={})
        data  = options[:data] || ""
        block = options[:want_result]

        @rpc_mutex.synchronize do
          @rpc_thread.perform(query_id, options[:request], data)

          if block
            Thread.current[:sunscraper_error] = nil
            @rpc_waiters[query_id] = Thread.current
          end
        end

        if block
          Thread.stop

          if error = Thread.current[:sunscraper_error]
            raise error
          end

          @rpc_results[query_id]
        end
      ensure
        if block
          @rpc_waiters.delete query_id
          @rpc_results.delete query_id
        end
      end
    end

    class RPCThread < Thread
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
        begin
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

          @creator.wakeup
        rescue Exception => e
          @creator[:sunscraper_error] = e
          return
        ensure
          server.close if server
          FileUtils.rm server_path if server_path

          @creator.wakeup
        end

        loop do
          begin
            header = @socket.read(4 * 3)
            query_id, request, data_length = header.unpack("NNN")
            data   = @socket.read(data_length) if data_length > 0

            @parent.rpc_mutex.synchronize do
              if !@parent.rpc_waiters.include?(query_id)
                $stderr.puts "Sunscraper-Standalone: no waiter for #{query_id}"
              else
                @parent.rpc_results[query_id] = data
                @parent.rpc_waiters[query_id].wakeup
              end
            end
          rescue Exception => e
            if thread = @parent.rpc_waiters[query_id]
              thread[:sunscraper_error] = e
              thread.wakeup
            else
              $stderr.puts "Sunscraper error detected outside of query context"
              $stderr.puts "#{e.class}: #{e.message}"
              e.backtrace.each do |line|
                $stderr.puts "  #{line}"
              end
            end

            break
          end
        end
      ensure
        @socket.close if @socket
        Process.kill pid if pid
      end
    end
  end
end