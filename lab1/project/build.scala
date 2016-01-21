import sbt._
import Keys._
import complete._
import complete.DefaultParsers._

object BuildSettings extends Build {
  override lazy val settings = super.settings ++ Seq(
    organization := "berkeley",
    version      := "1.2",
    scalaVersion := "2.11.6",
    parallelExecution in Global := false,
    traceLevel   := 15,
    scalacOptions ++= Seq("-deprecation","-unchecked"),
    libraryDependencies ++= Seq("org.scala-lang" % "scala-reflect" % scalaVersion.value)
  )

  lazy val chisel = (project in file("chisel"))
  lazy val sha3   = (project in file(".")).dependsOn(chisel)
}
