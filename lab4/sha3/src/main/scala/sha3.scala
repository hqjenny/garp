//see LICENSE for license
//authors: Colin Schmidt, Adam Izraelevitz
package sha3

//import Chisel.Implicits._
import Chisel._
import Chisel.AdvTester._

import rocket._

import cde.{Parameters, Field, Ex, World, ViewSym, Knob, Dump, Config, ParameterDump, Collector}
import cde.Implicits._

case object WidthP extends Field[Int]
case object Stages extends Field[Int]
case object FastMem extends Field[Boolean]
case object BufferSram extends Field[Boolean]

/*
abstract class SimpleRoCC()(implicit p: Parameters) extends RoCC()(p)
{
  io.interrupt := Bool(false)
    // Set this true to trigger an interrupt on the processor (please refer to supervisor documentation)

  //a simple accelerator doesn't use imem or page tables
  io.imem.acquire.valid := Bool(false)
  io.imem.grant.ready := Bool(true)
  io.dmem.head.acquire.valid := Bool(false)
  io.dmem.head.grant.ready := Bool(false)
  io.iptw.req.valid := Bool(false)
  io.dptw.req.valid := Bool(false)
  io.pptw.req.valid := Bool(false)
  io.mem.invalidate_lr := Bool(false)
}*/

abstract class SimpleRoCC()(implicit p: Parameters) extends RoCC()(p)
{
  io.interrupt := Bool(false)
    // Set this true to trigger an interrupt on the processor (please refer to supervisor documentation)

  //a simple accelerator doesn't use imem or page tables

  //Old Format
  //io.imem.acquire.valid := Bool(false)
  //io.imem.grant.ready := Bool(false)
  //io.imem.finish.valid := Bool(false)

  //New Format
  io.autl.acquire.valid := Bool(false)
  io.autl.grant.ready := Bool(false)
  io.iptw.req.valid := Bool(false)
  io.dptw.req.valid := Bool(false)
  io.pptw.req.valid := Bool(false)
}

class Sha3Accel()(implicit p: Parameters) extends SimpleRoCC()(p) {
  //parameters
  val W = p(WidthP)
  val S = p(Stages)
  //constants
  val r = 2*256
  val c = 25*W - r
  val round_size_words = c/W
  val rounds = 24 //12 + 2l
  val hash_size_words = 256/W
  val bytes_per_word = W/8

  //RoCC Interface defined in testMems.scala
  //cmd
  //resp
  io.resp.valid := Bool(false) //Sha3 never returns values with the resp
  //mem
  //busy

  val ctrl = Module(new CtrlModule(W,S)(p))
  
  ctrl.io.rocc_req_val   <> io.cmd.valid
  ctrl.io.rocc_req_rdy   <> io.cmd.ready
  ctrl.io.rocc_funct     <> io.cmd.bits.inst.funct
  ctrl.io.rocc_rs1       <> io.cmd.bits.rs1
  ctrl.io.rocc_rs2       <> io.cmd.bits.rs2
  ctrl.io.rocc_rd        <> io.cmd.bits.inst.rd
  ctrl.io.busy           <> io.busy

  ctrl.io.dmem_req_val   <> io.mem.req.valid
  ctrl.io.dmem_req_rdy   <> io.mem.req.ready
  ctrl.io.dmem_req_tag   <> io.mem.req.bits.tag
  ctrl.io.dmem_req_addr  <> io.mem.req.bits.addr
  ctrl.io.dmem_req_cmd   <> io.mem.req.bits.cmd
  ctrl.io.dmem_req_typ   <> io.mem.req.bits.typ

  ctrl.io.dmem_resp_val  <> io.mem.resp.valid
  ctrl.io.dmem_resp_tag  <> io.mem.resp.bits.tag
  ctrl.io.dmem_resp_data := io.mem.resp.bits.data

  val dpath = Module(new DpathModule(W,S))

  dpath.io.message_in <> ctrl.io.buffer_out
  io.mem.req.bits.data := dpath.io.hash_out(ctrl.io.windex)

  //ctrl.io <> dpath.io
  ctrl.io.absorb <> dpath.io.absorb
  ctrl.io.init <> dpath.io.init
  ctrl.io.write <> dpath.io.write
  ctrl.io.round <> dpath.io.round
  ctrl.io.stage <> dpath.io.stage
  ctrl.io.aindex <> dpath.io.aindex

}

class DefaultConfig() extends Config {
  override val topDefinitions:World.TopDefs = {
    (pname,site,here) => pname match {
      case WidthP => 64
      case Stages => Knob("stages")
      //case FastMem => Knob("fast_mem")
      case FastMem => true
      case BufferSram => Dump(Knob("buffer_sram"))
      //case "multi_vt" => Dump(Knob("multi_vt"))
    }
  }
  override val topConstraints:List[ViewSym=>Ex[Boolean]] = List(
    ex => ex(WidthP) === 64,
    ex => ex(Stages) >= 1 && ex(Stages) <= 4 && (ex(Stages)%2 === 0 || ex(Stages) === 1),
    //ex => ex(FastMem) === ex(FastMem),
    ex => ex(FastMem) === true,
    ex => ex(BufferSram) === ex(BufferSram)
    //ex => ex[Boolean]("multi_vt") === ex[Boolean]("multi_vt")
  )
  override val knobValues:Any=>Any = {
    case "stages" => 1
    //case "fast_mem" => true
    case "buffer_sram" => true
    //case "multi_vt" => true
  }
}

class Sha3AccelTests(c: Sha3Accel, isTrace:Boolean = false) extends AdvTester(c, isTrace) {
  // Setup the IO handlers
  val Cmd_IHandler     = new DecoupledSource(c.io.cmd, (sckt: RoCCCommand, in: TestCmd) => in.inject(this, sckt))
  val Resp_OHandler    = new DecoupledSink(c.io.resp, (sckt: RoCCResponse) => TestResp.extract(this, sckt))
  val MemReq_OHandler  = new DecoupledSink(c.io.mem.req, (sckt: HellaCacheReq) => TestMemReq.extract(this, sckt))
  val MemResp_IHandler = new ValidSource(c.io.mem.resp, (sckt: HellaCacheResp, in: TestMemResp) => in.inject(this, sckt))

  def dutBusy = peek(c.io.busy) == 1

  // Setup some memory models
  val instantMemory = new InstantMemory(64, 64*64*2, MemReq_OHandler.outputs, MemResp_IHandler.inputs)
  val slowMemory = new SlowMemory(64, 64*64*2, MemReq_OHandler.outputs, MemResp_IHandler.inputs)
  val randomMemory = new RandomMemory(64, 64*64*2, MemReq_OHandler.outputs, MemResp_IHandler.inputs)
  val testMemory = slowMemory
  //This test is the same as the one in the keccak code on github
  //https://github.com/gvanas/KeccakCodePackage/blob/master/TestVectors/KeccakF-1600-IntermediateValues.txt
  val test_message = Array.fill(c.round_size_words){BigInt(0)}
  val test_hash = Array(BigInt("F1258F7940E1DDE7",16),
                        BigInt("84D5CCF933C0478A",16),
                        BigInt("D598261EA65AA9EE",16),
                        BigInt("BD1547306F80494D",16))

  takestep()
  takestep()
  takestep()
  //Setup memory
  testMemory.store_data(0, Array.fill(17){BigInt(0)})
  
  //insert both RoCC commands
  Cmd_IHandler.inputs.enqueue(TestCmd.addr(0,1024))
  Cmd_IHandler.inputs.enqueue(TestCmd.len(17*8))
  //run test
  until( Cmd_IHandler.isIdle && !dutBusy, 250 ) { testMemory.process() }
  
  //check answer
  var resp = testMemory.load_addr(1024)
  assert(resp == test_hash(0), "Test failed word 0: Bad response (got %d, expected %d)".format(resp,test_hash(0)) )
  resp = testMemory.load_addr(1024+8*1)
  assert(resp == test_hash(1), "Test failed word 1: Bad response (got %d, expected %d)".format(resp,test_hash(1)) )
  resp = testMemory.load_addr(1024+8*2)
  assert(resp == test_hash(2), "Test failed word 2: Bad response (got %d, expected %d)".format(resp,test_hash(2)) )
  resp = testMemory.load_addr(1024+8*3)
  assert(resp == test_hash(3), "Test failed word 3: Bad response (got %d, expected %d)".format(resp,test_hash(3)) )

  //Setup memory
  testMemory.store_data(256, Array.fill(17){BigInt(0)})
  //insert both RoCC commands
  Cmd_IHandler.inputs.enqueue(TestCmd.addr(256,1152))
  Cmd_IHandler.inputs.enqueue(TestCmd.len(17*8))
  //run test
  until( Cmd_IHandler.isIdle && !dutBusy, 250 ) { testMemory.process() }
  
  //check answer
  resp = testMemory.load_addr(1152)
  assert(resp == test_hash(0), "Test failed word 0: Bad response (got %d, expected %d)".format(resp,test_hash(0)) )
  resp = testMemory.load_addr(1152+8*1)
  assert(resp == test_hash(1), "Test failed word 1: Bad response (got %d, expected %d)".format(resp,test_hash(1)) )
  resp = testMemory.load_addr(1152+8*2)
  assert(resp == test_hash(2), "Test failed word 2: Bad response (got %d, expected %d)".format(resp,test_hash(2)) )
  resp = testMemory.load_addr(1152+8*3)
  assert(resp == test_hash(3), "Test failed word 3: Bad response (got %d, expected %d)".format(resp,test_hash(3)) )

    println("Test 1: Execute empty message hash")
    var start_cycles = cycles
    testMemory.reqs_handled = 0 
    Cmd_IHandler.inputs.enqueue(TestCmd.addr(0, 1024))
    Cmd_IHandler.inputs.enqueue(TestCmd.len(150))
    until( Cmd_IHandler.isIdle && !dutBusy, 500 ) {
      //peek(c.ctrl.mem_s)
      //peek(c.ctrl.state)
      //peek(c.ctrl.io.buffer_out)
      //peek(c.ctrl.buffer_count)
      //peek(c.ctrl.buffer_valid)
      //peek(c.ctrl.hashed)
      //peek(c.ctrl.io.absorb)
      //peek(c.ctrl.mindex)
      //peek(c.ctrl.read)
      //peek(c.ctrl.msg_len)
      testMemory.process() }
    println("Test 1 took "+(cycles-start_cycles)+" cycles and had "+testMemory.reqs_handled+" requests for a util of "+testMemory.reqs_handled/(cycles-start_cycles).toDouble)
    var t1_resp = testMemory.load_addr(1024)
    t1_resp = testMemory.load_addr(1024)
    assert( t1_resp == BigInt("1f56d343d99dccdd", 16), "Test 1 failed: Bad response (got %d, expected %d)".format(t1_resp, BigInt("1f56d343d99dccdd", 16)) )
    t1_resp = testMemory.load_addr(1032)
    assert( t1_resp == BigInt("1ac1c261f52ca836", 16), "Test 1 failed: Bad response (got %d, expected %d)".format(t1_resp, BigInt("1ac1c261f52ca836", 16)) )
    t1_resp = testMemory.load_addr(1040)
    assert( t1_resp == BigInt("ae278642a6872aea", 16), "Test 1 failed: Bad response (got %d, expected %d)".format(t1_resp, BigInt("ae278642a6872aea", 16)) )
    t1_resp = testMemory.load_addr(1048)
    assert( t1_resp == BigInt("ee392a8995033db8", 16), "Test 1 failed: Bad response (got %d, expected %d)".format(t1_resp, BigInt("ee392a8995033db8", 16)) )

/* TODO: still one bug
//Long bmark
    testMemory.store_data(0, Array.fill(1024){BigInt("3736353433323130",16)})
    start_cycles = cycles
    testMemory.reqs_handled = 0 
    Cmd_IHandler.inputs.enqueue(TestCmd.addr(0, 8192))
    Cmd_IHandler.inputs.enqueue(TestCmd.len(1024*8))
    until( Cmd_IHandler.isIdle && !dutBusy, 5000 ) { 
  peek(c.io.cmd.ready); peek(c.io.mem.req.bits.data)
  
  peek(c.ctrl.state);
  peek(c.io.cmd.ready);
  peek(c.io.mem.req.bits.data)
  peek(c.dpath.io.hash_out)
  peek(c.ctrl.msg_addr);
  peek(c.ctrl.hash_addr);
  peek(c.ctrl.msg_len);
  peek(c.ctrl.busy);
  peek(c.ctrl.mem_s);
  peek(c.ctrl.rocc_s);
  peek(c.ctrl.buffer_valid);
  peek(c.ctrl.rindex);
  peek(c.dpath.io.stage);
  peek(c.ctrl.hashed);
  peek(c.ctrl.windex);
  peek(c.ctrl.writes_done);
  peek(c.io.mem.req.bits.tag);
  peek(c.ctrl.mindex);
  peek(c.ctrl.read);
      
      testMemory.process() }
    println("Test 12 took "+(cycles-start_cycles)+" cycles and had "+testMemory.reqs_handled+" requests for a util of "+testMemory.reqs_handled/(cycles-start_cycles).toDouble)
    var t12_resp = testMemory.load_addr(8192)
    assert( t12_resp == BigInt("d667643f4e4150d1", 16), "Test 12(1,1) failed: Bad response (got %d, expected %d)".format(t12_resp, BigInt("d667643f4e4150d1", 16)) )
    t12_resp = testMemory.load_addr(8200)
    assert( t12_resp == BigInt("e3fdc3aaddec642e", 16), "Test 12(1,2) failed: Bad response (got %d, expected %d)".format(t12_resp, BigInt("e3fdc3aaddec642e", 16)) )
    t12_resp = testMemory.load_addr(8208)
    assert( t12_resp == BigInt("043bb6a24a6c6f82", 16), "Test 12(1,3) failed: Bad response (got %d, expected %d)".format(t12_resp, BigInt("043bb6a24a6c6f82", 16)) )
    t12_resp = testMemory.load_addr(8216)
    assert( t12_resp == BigInt("3f316f48ce20b353", 16), "Test 12(1,4) failed: Bad response (got %d, expected %d)".format(t12_resp, BigInt("3f316f48ce20b353", 16)) )
    */
}

object Sha3AccelConfigCollect extends App with FileSystemUtilities{
  //Normally, the --configCollect option would dump the configs and knobs to a file
  //This is not completly push button with CDE yet.
  //This class essentially recreates the logic for --configCollect but for CDE
 
  //We pass in some positional arguments to make things easier
  //This should be integrated with the CLI flags at some point
  //but is how rocket-chip accomplishes this
  val projectName = args(0)
  val topModuleName = args(1)
  val configClassName = args(2)
  val target = args(3)

  //This is a bit of reflection to find and instantiate the right configuration
  val config = try {
    Class.forName(s"$projectName.$configClassName").newInstance.asInstanceOf[Config]
  }catch{
    case e: java.lang.ClassNotFoundException =>
    throwException(s"Could not find the cde.Config subclass you asked for " +
                      "(i.e. \"$configClassName\"), did you misspell it?", e)
  }

  //The collector is used in the first pass to collect all parameters
  val world = new Collector(config.topDefinitions, config.knobValues)

  //Apply each constraint.  Without this, nothing will be outputed
  //Note that the collector has mutible state (not pure functional)
  val p = Parameters.root(world)
  config.topConstraints.foreach(c => p.constrain(c))

  //For Quick Debug
  println("Knobs: " + world.getKnobs)
  println("Constraints: " + world.getConstraints)
  
  //Output to Files
  val v = createOutputFile(target + "/" + configClassName + ".knb")
  v.write(world.getKnobs)
  v.close
  val w = createOutputFile(target + "/" + configClassName + ".cst")
  w.write(world.getConstraints)
  w.close
}


object Sha3AccelMain{
  def main(args: Array[String]): Unit = {
    //This function instantiates the tester with a specified config
    
    //We pass in some positional arguments to make things easier
    //This should be integrated with the CLI flags at some point
    //but is how rocket-chip accomplishes this
    val projectName = args(0)
    val topModuleName = args(1)
    val configClassName = args(2)
    println("ConfigClass: "+projectName+"."+configClassName)
    //val target = args(3)
    val config = try {
      Class.forName(s"$projectName.$configClassName").newInstance.asInstanceOf[Config]
    }catch{
      case e: java.lang.ClassNotFoundException =>
      throwException(s"Could not find the cde.Config subclass you asked for " +
                        "(i.e. \"$configClassName\"), did you misspell it?", e)
    }

    val world = config.toInstance
    val paramsFromConfig: Parameters = Parameters.root(world)

    //Set isTrace to true to enable peek/poke printing
    chiselMainTest(args.drop(3), () => Module(new Sha3Accel()(paramsFromConfig))){c => new Sha3AccelTests(c, isTrace = false)}
  }
}
