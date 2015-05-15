package com.vmware

import akka.actor._
import akka.kernel.Bootable
import com.typesafe.config.ConfigFactory

case class Tenant(user:String, password:String, cmd:String)

class VmDirActor extends Actor {
  def receive = {
	case Tenant(user, password, cmd)=> VmDir.CreateTenant(user, password, cmd)
    case message: String =>
      println("Received message '%s'" format message)
  }
}

class RemoteKernel extends Bootable {
  private val system = ActorSystem("remotekernel", ConfigFactory.load().getConfig("RemoteSys"))

  def startup = {
    system.actorOf(Props[VmDirActor], "vmdir") ! "Hello world!"
  }

  def shutdown = {
    system.shutdown()
  }
}
