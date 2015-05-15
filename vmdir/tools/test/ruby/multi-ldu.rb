require 'net/ssh'

def ssh(ip, username, password)
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

def merge
  ssh(LDU2,OS_USER,OS_PASSWORD) do |ssh|
    if OS=='win'
      mergeBin='C:\Program Files\VMware\CIS\vmdird\vdcmerge.exe'
    else
      mergeBin='/usr/lib/vmware-vmdir/bin/vdcmerge'
    end
    cmd = "#{mergeBin} -u %s -w %s -H %s -U %s -W %s" % [LOTUS_USERNAME,LOTUS_PASSWORD,LDU1,LOTUS_USERNAME,LOTUS_PASSWORD]
    ssh_exec!(ssh, cmd)
  end
end

def split
  ssh(LDU2,OS_USER,OS_PASSWORD) do |ssh|
    if OS=='win'
      splitBin='C:\Program Files\VMware\CIS\vmdird\vdcsplit.exe'
    else
      splitBin='/usr/lib/vmware-vmdir/bin/vdcsplit'
    end
    cmd = "#{splitBin} -u %s -w %s -D %s -U %s -W %s" % [LOTUS_USERNAME,LOTUS_PASSWORD,LOTUS_DOMAIN,LOTUS_USERNAME,LOTUS_PASSWORD]
    output, exit_code=ssh_exec!(ssh, cmd)
  end
end

LOTUS_DOMAIN='vsphere.local'
LOTUS_USERNAME='administrator'
LOTUS_PASSWORD='vmware'

p ARGV

OS=ARGV[0]
LDU1=ARGV[1]
LDU2=ARGV[2]

if OS=='win'
  OS_USER='administrator'
  OS_PASSWORD='ca$hc0w'
else
  OS_USER='root'
  OS_PASSWORD='vmware'
end

merge
split
sleep 60
merge
split


