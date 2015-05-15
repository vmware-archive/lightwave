#!/usr/bin/expect

set timeout -1

if { $argc != 4 } {
    puts "Usage $argv0 host user pass command"
    puts "For example: $argv0 10.151.142.53 root vmware 'uname -a'"
    exit 1
}

set host [lindex $argv 0]
set user [lindex $argv 1]
set pass [lindex $argv 2]
set command [lindex $argv 3]

spawn ssh -oUserKnownHostsFile=/dev/null -oStrictHostKeyChecking=no -oCheckHostIP=no $user@$host $command

expect *assword:
send "$pass\r"
expect eof
