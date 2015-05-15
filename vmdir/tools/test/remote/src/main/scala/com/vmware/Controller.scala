package com.vmware

import akka.actor._
import com.typesafe.config.ConfigFactory

object Controller extends App {
  val system = ActorSystem("Controller", ConfigFactory.load().getConfig("Host"))
  val remote = system.actorFor("akka://remotekernel@192.168.103.131:2552/user/vmdir")

  for(i<-0 to 10000){
	  remote ! Tenant("administrator", "vmware", "tenant"+i)
  }
  system.shutdown()
}
