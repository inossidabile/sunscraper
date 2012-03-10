require 'socket'

# @private
module Sunscraper
  module Standalone
    @rpc_mutex  = Mutex.new
    @rpc_socket = nil

    RPC_LOAD_URL  = 1
    RPC_LOAD_HTML = 2
    RPC_WAIT      = 3
    RPC_FETCH     = 4

    class << self
      def create
        connect_to_worker
      end

      def load_url(socket, url)
        perform_rpc socket,
          request:     RPC_LOAD_URL,
          data:        url
      end

      def load_html(socket, html, baseUrl)
        html, baseUrl = [html, baseUrl].map(&:to_s)
        perform_rpc socket,
          request:     RPC_LOAD_HTML,
          data:        [html.length, html, baseUrl.length, baseUrl].pack("Na*Na*")
      end

      def wait(socket, timeout)
        result = perform_rpc socket,
          request:     RPC_WAIT,
          data:        [timeout].pack("N"),
          want_result: true
        code, = result.unpack("N")

        code == 1 # true
      end

      def fetch(socket)
        perform_rpc socket,
          request:     RPC_FETCH,
          want_result: true
      end

      def finalize(socket)
        socket.close
      end

      private

      def perform_rpc(socket, options={})
        data  = options[:data] || ""
        socket.write([options[:request], data.length, data].pack("NNa*"))

        if options[:want_result]
          data_length, = socket.read(4).unpack("N")
          socket.read(data_length)
        end
      end

      def spawn_worker
        if ::Sunscraper.os_x?
          # Fuck you, OS X.
          suffix = ".app/Contents/MacOS/sunscraper"
        else
          suffix = RbConfig::CONFIG["EXEEXT"]
        end

        executable = File.join(Gem.loaded_specs['sunscraper'].full_gem_path,
                          'ext', 'standalone', "sunscraper#{suffix}")

        server_path = "/tmp/sunscraper.#{Process.pid}.sock"
        File.unlink server_path if File.exists? server_path

        if Kernel.respond_to? :spawn
          @rpc_pid = Kernel.spawn "#{executable} #{server_path}"
        else
          # rbx does not have Kernel.spawn (yet). Sigh...
          @rpc_pid = fork { exec executable, server_path }
        end

        # Sigh again. Probably no other way.
        loop do
          if File.exists? server_path
            @rpc_socket = server_path
            break
          elsif Process.wait(@rpc_pid, Process::WNOHANG)
            raise RuntimeError, "Cannot start Sunscraper process"
          end

          sleep 0.1
        end

        Process.detach @rpc_pid

        at_exit do
          Process.kill "KILL", @rpc_pid
          File.unlink @rpc_socket
        end
      end

      def connect_to_worker
        @rpc_mutex.synchronize do
          spawn_worker if @rpc_socket.nil?
        end

        UNIXSocket.new(@rpc_socket)
      end
    end
  end
end