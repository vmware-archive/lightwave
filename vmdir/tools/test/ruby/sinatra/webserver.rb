require 'rubygems'
require 'sinatra'
require 'haml'
require 'net/ssh'
set :bind, '0.0.0.0'

get '/hi' do
    "Hello World!"
end

get '/' do
    haml :index
end

get '/merge' do
    haml :merge
end

get '/split' do
    haml :split
end

post '/merge' do
    src = params[:source]
    tgt = params[:target]
    os  = params[:os_type]
    "Merging #{src} to #{tgt}. OS type = #{os}"

    ldu1=tgt
    ldu2=src

    os_user = admin_user(os)
    os_pwd  = admin_pwd(os)
    os_type = os

    ldap_user = lotus_username()
    ldap_pwd = lotus_password()
    ldap_domain = lotus_domain()

    puts("merge #{os_user} #{os_pwd} #{os_type} #{ldap_user} #{ldap_pwd} #{ldu1} #{ldu2}")
    merge(os_user, os_pwd, os_type, ldap_user, ldap_pwd, ldu1, ldu2)

end

post '/split' do
    src = params[:source]
    os  = params[:os_type]
    "Spliting #{src}. OS type = #{os}"

    ldu2=src

    os_user = admin_user(os)
    os_pwd  = admin_pwd(os)
    os_type = os

    ldap_user = lotus_username()
    ldap_pwd = lotus_password()
    ldap_domain = lotus_domain()

    puts("split #{os_user} #{os_pwd} #{os_type} #{ldap_user} #{ldap_pwd} #{ldu2}")
    split(os_user, os_pwd, os_type, ldap_user, ldap_pwd, ldu2)

end

def admin_user(os_type)
    if os_type=='windows'
      return 'administrator'
    else
      return 'root'
    end
end

def admin_pwd(os_type)
    if os_type=='windows'
      return 'ca$hc0w'
    else
      return 'vmware'
    end
end

def lotus_domain
      'vsphere.local'
end

def lotus_username
      'administrator'
end

def lotus_password
      'vmware'
end

def ssh(ip, username, password)
    p ip
    p username
    p password
    sshParams = {
      password: password,
      paranoid: false,
      verbose: :warn,
      auth_methods: %w(keyboard-interactive password),
      port: 22
    }
    Net::SSH.start ip, username, sshParams do |ssh|
      yield ssh
    end
end

def ssh_exec!(ssh, command)
    puts command
    stdout_data = ""
    exit_code = nil
    ssh.open_channel do |channel|
      channel.exec(command) do |ch, success|
        unless success
          raise "FAILED: couldn't execute command (ssh.channel.exec)"
        end
        channel.on_data do |ch,data|
          stdout_data+=data
        end

        channel.on_request("exit-status") do |ch,data|
          exit_code = data.read_long
        end
      end
    end
    ssh.loop
    puts stdout_data
    raise "Running VC merge-tool failed. Error:#{exit_code}" unless exit_code==0
end

def merge_bin(os_type)
    if os_type=='windows'
      return 'C:\Program Files\VMware\CIS\vmdird\vdcmerge.exe'
    else
      return '/usr/lib/vmware-vmdir/bin/vdcmerge'
    end
end

def split_bin(os_type)
    if os_type=='windows'
      return 'C:\Program Files\VMware\CIS\vmdird\vdcsplit.exe'
    else
      return '/usr/lib/vmware-vmdir/bin/vdcsplit'
    end
end

def merge(os_user, os_pwd, os_type, ldap_user, ldap_pwd, ldu1, ldu2)
    ssh(ldu2,os_user,os_pwd) do |ssh|
      mergeBin = merge_bin(os_type)
      cmd = "#{mergeBin} -u %s -w %s -H %s -U %s -W %s" % [lotus_username,lotus_password,ldu1,lotus_username,lotus_password]
      ssh_exec!(ssh, cmd)
    end
end

def split(os_user, os_pwd, os_type, ldap_user, ldap_pwd, ldu2)
    ssh(ldu2,os_user,os_pwd) do |ssh|
      splitBin = split_bin(os_type)
      cmd = "#{splitBin} -u %s -w %s -D %s -U %s -W %s" % [lotus_username,lotus_password,lotus_domain,lotus_username,lotus_password]
      output, exit_code=ssh_exec!(ssh, cmd)
    end
end
