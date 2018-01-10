require 'rubygems'
require 'sinatra'
set :port, 82
before do
    request.body.rewind
    @request_payload = request.body.read
end
post '/lightwaveui/Home' do
   redirect "http://172.16.127.222" + request.fullpath+'?'+@request_payload
end

get '/' do
    'hello world'
end
