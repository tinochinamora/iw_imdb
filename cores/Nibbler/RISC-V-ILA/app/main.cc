#include "riscvIla.hpp"

using namespace ilang;

/// the function to parse commandline arguments
VerilogVerificationTargetGenerator::vtg_config_t HandleArguments(int argc, char **argv);

/// To verify the exact AES128 ILA
void verifyNibbler(
  Ila& model, 
  VerilogVerificationTargetGenerator::vtg_config_t vtg_cfg,
  const std::vector<std::string> & design_files,
  const std::string & varmap,
  const std::string & instcont
  ) {
  VerilogGeneratorBase::VlgGenConfig vlg_cfg; 
  vlg_cfg.pass_node_name = true;
  vtg_cfg.CosaAddKeep = false;


  std::string RootPath = "..";
  std::string VerilogPath = RootPath + "/verilog/";
  std::string IncludePath = VerilogPath + "include/";
  std::string RefrelPath = RootPath + "/refinement/";
  std::string OutputPath = RootPath + "/verification/";

  std::vector<std::string> path_to_design_files; // update path
  for(auto && f : design_files)
    path_to_design_files.push_back( VerilogPath + f );

  VerilogVerificationTargetGenerator vg(
      {},                             // no include
      path_to_design_files,           // designs
      "param-Core",                      // top_module_name
      RefrelPath + varmap,            // variable mapping
      RefrelPath + instcont,          // conditions of start/ready
      OutputPath,                     // output path
      model.get(),                    // model
      VerilogVerificationTargetGenerator::backend_selector::COSA, // backend: COSA
      vtg_cfg,  // target generator configuration
      vlg_cfg); // verilog generator configuration

  vg.GenerateTargets();
}


int main() {
  // TODO

  std::vector<std::string> design_files = {
    "param-ClkEnBuf.v",
    "param-Core.v",
    "param-CoreDpathAlu.v",
    "param-CoreDpathRegfile.v",
    "param-Ctrl.v",
    "param-DeserializedReg.v",
    "param-Dpath.v",
    "param-PCComputation.v",
    "param-ShiftDemux.v",
    "param-SIMDLaneDpath.v"
  };

  //auto vtg_cfg = SetConfiguration();
  auto vtg_cfg = HandleArguments(argc, argv);

  // build the model
  EmeshAxiMasterBridge emaxi;

  verifyNibbler(emaxi.wmodel, vtg_cfg, design_files, "varmap-nibbler.json", "instcond-nibbler.json");

  riscvILA_user riscvILA(0);
  riscvILA.addInstructions(); // 37 base integer instructions

  return 0;
}

VerilogVerificationTargetGenerator::vtg_config_t HandleArguments(int argc, char **argv) {
  // the solver, the cosa environment
  // you can use a commandline parser if desired, but since it is not the main focus of
  // this demo, we skip it

  // set ilang option, operators like '<' will refer to unsigned arithmetics
  SetUnsignedComparison(true); 
  
  VerilogVerificationTargetGenerator::vtg_config_t ret;

  for(unsigned p = 1; p<argc; p++) {
    std::string arg = argv[p];
    auto split = arg.find("=");
    auto argName = arg.substr(0,split);
    auto param   = arg.substr(split+1);

    if(argName == "Solver")
      ret.CosaSolver = param;
    else if(argName == "Env")
      ret.CosaPyEnvironment = param;
    else if(argName == "Cosa")
      ret.CosaPath = param;
    // else unknown
    else {
      std::cerr<<"Unknown argument:" << argName << std::endl;
      std::cerr<<"Expecting Solver/Env/Cosa=???" << std::endl;
    }
  }

  ret.CosaGenTraceVcd = true;

  /// other configurations
  ret.PortDeclStyle = VlgVerifTgtGenBase::vtg_config_t::NEW;
  ret.CosaGenJgTesterScript = true;
  //ret.CosaOtherSolverOptions = "--blackbox-array";
  //ret.ForceInstCheckReset = true;

  return ret;
}

VerilogVerificationTargetGenerator::vtg_config_t SetConfiguration() {

  // set ilang option, operators like '<' will refer to unsigned arithmetics
  SetUnsignedComparison(true); 
  
  VerilogVerificationTargetGenerator::vtg_config_t ret;
  ret.CosaSolver = "btor";
  ret.CosaPyEnvironment = "~/cosaEnv/bin/activate";
  ret.CosaPath = "~/CoSA";
  ret.CosaGenTraceVcd = true;

  /// other configurations
  ret.PortDeclStyle = VlgVerifTgtGenBase::vtg_config_t::NEW;
  ret.CosaGenJgTesterScript = true;
  //ret.CosaOtherSolverOptions = "--blackbox-array";
  //ret.ForceInstCheckReset = true;

  return ret;
}

