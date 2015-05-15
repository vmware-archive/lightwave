import sbt._
import sbt.Keys._
import akka.sbt.AkkaKernelPlugin
import akka.sbt.AkkaKernelPlugin.{ Dist, outputDirectory, distJvmOptions}

object TestBuild extends Build {

  lazy val test = Project(
    id = "remote",
    base = file("."),
    settings = Project.defaultSettings ++ AkkaKernelPlugin.distSettings ++ Seq(
      name := "remote",
      organization := "vmware.com",
      version := "2.1.2",
      scalaVersion := "2.9.2",
      resolvers += "Typesafe Releases" at "http://repo.typesafe.com/typesafe/releases",
      libraryDependencies += "com.typesafe.akka" % "akka-actor" % "2.0.1",
      libraryDependencies += "com.typesafe.akka" % "akka-kernel" % "2.0.1",
      libraryDependencies += "com.typesafe.akka" % "akka-remote" % "2.0.1",
      libraryDependencies += "com.typesafe.akka" % "akka-slf4j" % "2.0.1",
      libraryDependencies += "ch.qos.logback" % "logback-classic" % "1.0.0",
      distJvmOptions in Dist := "-Xms256M -Xmx1024M"
    )
  )
}
