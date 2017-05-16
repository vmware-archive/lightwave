#!/usr/bin/env ruby
# encoding: utf-8

require 'json'
require 'optparse'
require 'pp'
require 'rest-client'
require 'yaml'

class User
  attr_reader :name
  attr_reader :password
  attr_reader :groups

  def initialize(args)
    args.each do |key, value|
      instance_variable_set("@#{key}", value) unless value.nil?
    end
  end
end

class Group
  attr_reader :name
  attr_reader :description

  def initialize(name, description)
    @name = name
    @description = description
  end
end

class Client
  attr_reader :additional_information
  attr_reader :authorities
  attr_reader :authorized_grant_types
  attr_reader :autoapprove
  attr_reader :id
  attr_reader :redirect_uris
  attr_reader :resource_ids
  attr_reader :scopes
  attr_reader :secret

  def initialize(id, args)
    @id = id
    args.each do |key, value|
      instance_variable_set("@#{key}", value) unless value.nil?
    end
  end
end

class IdmClient

  attr_reader :username
  attr_reader :password
  attr_reader :host
  attr_reader :tenant

  def initialize(args)
    args.each do |key, value|
      instance_variable_set("@#{key}", value) unless value.nil?
    end
    @token = get_token
  end

  def get_token
    payload = {
      grant_type: "password",
      username: "#{@username}@#{@tenant}",
      password: @password,
      scope: "openid offline_access id_groups at_groups rs_admin_server"
    }
    r = create_resource("https://#{@host}/openidconnect/token/").post(payload)
    @token = JSON.parse(r.body)["access_token"]
  end

  def create_group(group)
    payload = {
      name: group.name,
      domain: @tenant,
      details: {
        description: group.description
      }
    }
    create_resource("https://#{@host}/vmdir/tenant/#{@tenant}/groups").post(payload.to_json, create_header)
  end

  def delete_group(group)
    create_resource("https://#{@host}/vmdir/tenant/#{@tenant}/groups/#{group.name}@#{@tenant}").delete(create_header)
  end

  def create_user(user)
    payload = {
      name: user.name,
      domain: @tenant,
      disabled: false,
      locked: false,
      details: {
        description: user.name,
        email: "#{user.name}@#{@tenant}",
        upn: "#{user.name}@#{@tenant}"
      },
      passwordDetails: {
        password: user.password
      }
    }
    create_resource("https://#{@host}/vmdir/tenant/#{tenant}/users/").post(payload.to_json, create_header)

    user.groups.each do |group|
      query = { members: "#{user.name}@#{@tenant}", type: "user" }
      create_resource("https://#{@host}/vmdir/tenant/#{@tenant}/groups/#{group}@#{@tenant}/members?#{URI.encode_www_form(query)}").put(nil, create_header)
    end
  end

  def delete_user(user)
    create_resource("https://#{@host}/vmdir/tenant/#{@tenant}/users/#{user.name}@#{@tenant}").delete(create_header)
  end

  def create_client(client)
    payload = {
      clientId: client.id,
      oidcclientMetadataDTO: {
        redirectUris: client.redirect_uris,
        clientSecret: client.secret,
        authorities: client.authorities,
        resourceIds: client.resource_ids,
        scopes: client.scopes,
        autoApproveScopes: client.autoapprove,
        authorizedGrantTypes: client.authorized_grant_types,
        additionalInformation: client.additional_information
      }
    }
    create_resource("https://#{@host}/idm/tenant/#{@tenant}/oidcclient/").post(payload.to_json, create_header)
  end

  def delete_client(client)
    create_resource("https://#{@host}/idm/tenant/#{@tenant}/oidcclient/#{client.id}/").delete(create_header)
  end

  def create_header
    { content_type: :json, accept: :json, Authorization: "Bearer #{@token}" }
  end

  def create_resource(url)
    RestClient::Resource.new(url, :verify_ssl => OpenSSL::SSL::VERIFY_NONE, log: Logger.new(STDERR))
  end

  private :get_token, :create_header, :create_resource
end

def action(arg)
  arg ? "Deleting" : "Creating"
end

options = {}
OptionParser.new do |opts|
  opts.banner = "Usage: createFixtures.rb <filename> [options]"

  opts.on("-u", "--username USERNAME", "The username to to connect to IDM with. REQUIRED.") do |username|
    options[:username] = username
  end

  opts.on("-p", "--password PASSWORD", "The password to connect to IDM with. REQUIRED.") do |password|
    options[:password] = password
  end

  opts.on("-t", "--tenant TENANT", "The tenant to generate the fixtures on. REQUIRED.") do |tenant|
    options[:tenant] = tenant
  end

  opts.on("-h", "--host HOST", "The host to connect to. REQUIRED.") do |host|
    options[:host] = host
  end

  opts.on("-d", "--delete", "Delete the fixtures rather than create them.") do |delete|
    options[:delete] = true
  end
end.parse!

raise OptionParser::MissingArgument.new("username") if options[:username].nil?
raise OptionParser::MissingArgument.new("password") if options[:password].nil?
raise OptionParser::MissingArgument.new("tenant") if options[:tenant].nil?
raise OptionParser::MissingArgument.new("host") if options[:host].nil?

filename = ARGV.pop
raise "Need to specify a YML file containing the fixtures to create" unless filename
data = YAML.load(File.open(filename))

idm_client = IdmClient.new(options)

begin
  puts "------- #{action(options[:delete])} Groups -------"
  data["groups"].each do |name, description|
    group = Group.new(name, description)
    puts "#{group.pretty_inspect}"
    if options[:delete] then
      idm_client.delete_group(group)
    else
      idm_client.create_group(group)
    end
  end

  puts ""
  puts "------- #{action(options[:delete])} Users -------"
  data["users"].each do |userYml|
    user = User.new(userYml)
    puts "#{user.pretty_inspect}"
    if options[:delete] then
      idm_client.delete_user(user)
    else
      idm_client.create_user(user)
    end
  end

  puts ""
  puts "------- #{action(options[:delete])} Clients -------"
  data["clients"].each do |id, clientYml|
    client = Client.new(id, clientYml)
    puts "#{client.pretty_inspect}"
    if options[:delete] then
      idm_client.delete_client(client)
    else
      idm_client.create_client(client)
    end
  end
rescue RestClient::ExceptionWithResponse => e
  STDERR.puts "Error #{e.message}"
  STDERR.puts e.response
end
