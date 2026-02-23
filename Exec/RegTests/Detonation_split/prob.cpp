// Exec/RegTests/Detonation1D/prob.cpp
#include "prob.H"
#include <AMReX_ParmParse.H>
#include <string>

using namespace amrex;

std::string
read_pmf_file(std::ifstream& in)
{
  return static_cast<std::stringstream const&>(
           std::stringstream() << in.rdbuf())
    .str();
}

bool
checkQuotes(const std::string& str)
{
  int count = 0;
  for (char c : str) {
    if (c == '"') {
      count++;
    }
  }
  return (count % 2) == 0;
}

void
read_pmf(const std::string& myfile)
{
  std::string firstline;
  std::string secondline;
  std::string remaininglines;
  unsigned int pos1;
  unsigned int pos2;
  int variable_count;
  int line_count;

  std::ifstream infile(myfile);
  const std::string memfile = read_pmf_file(infile);
  infile.close();
  std::istringstream iss(memfile);

  std::getline(iss, firstline);
  if (!checkQuotes(firstline)) {
    amrex::Abort("PMF file variable quotes unbalanced");
  }
  std::getline(iss, secondline);
  pos1 = 0;
  pos2 = 0;
  variable_count = 0;
  while ((pos1 < firstline.length() - 1) && (pos2 < firstline.length() - 1)) {
    pos1 = firstline.find('"', pos1);
    pos2 = firstline.find('"', pos1 + 1);
    variable_count++;
    pos1 = pos2 + 1;
  }

  amrex::Vector<std::string> pmf_names;
  pmf_names.resize(variable_count);
  pos1 = 0;
  // pos2 = 0;
  for (int i = 0; i < variable_count; i++) {
    pos1 = firstline.find('"', pos1);
    pos2 = firstline.find('"', pos1 + 1);
    pmf_names[i] = firstline.substr(pos1 + 1, pos2 - (pos1 + 1));
    pos1 = pos2 + 1;
  }

  amrex::Print() << variable_count << " variables found in PMF file"
                 << std::endl;
  // for (int i = 0; i < variable_count; i++)
  //  amrex::Print() << "Variable found: " << pmf_names[i] <<
  //  std::endl;

  line_count = 0;
  while (std::getline(iss, remaininglines)) {
    line_count++;
  }
  amrex::Print() << line_count << " data lines found in PMF file" << std::endl;

  PeleC::h_prob_parm_device->pmf_N = line_count;
  PeleC::h_prob_parm_device->pmf_M = variable_count - 1;
  PeleC::prob_parm_host->h_pmf_X.resize(PeleC::h_prob_parm_device->pmf_N);
  PeleC::prob_parm_host->pmf_X.resize(PeleC::h_prob_parm_device->pmf_N);
  PeleC::prob_parm_host->h_pmf_Y.resize(
    PeleC::h_prob_parm_device->pmf_N * PeleC::h_prob_parm_device->pmf_M);
  PeleC::prob_parm_host->pmf_Y.resize(
    PeleC::h_prob_parm_device->pmf_N * PeleC::h_prob_parm_device->pmf_M);

  iss.clear();
  iss.seekg(0, std::ios::beg);
  std::getline(iss, firstline);
  std::getline(iss, secondline);
  for (unsigned int i = 0; i < PeleC::h_prob_parm_device->pmf_N; i++) {
    std::getline(iss, remaininglines);
    std::istringstream sinput(remaininglines);
    sinput >> PeleC::prob_parm_host->h_pmf_X[i];
    for (unsigned int j = 0; j < PeleC::h_prob_parm_device->pmf_M; j++) {
      sinput >> PeleC::prob_parm_host
                  ->h_pmf_Y[j * PeleC::h_prob_parm_device->pmf_N + i];
    }
  }

  amrex::Gpu::copy(
    amrex::Gpu::hostToDevice, PeleC::prob_parm_host->h_pmf_X.begin(),
    PeleC::prob_parm_host->h_pmf_X.end(), PeleC::prob_parm_host->pmf_X.begin());
  amrex::Gpu::copy(
    amrex::Gpu::hostToDevice, PeleC::prob_parm_host->h_pmf_Y.begin(),
    PeleC::prob_parm_host->h_pmf_Y.end(), PeleC::prob_parm_host->pmf_Y.begin());
  PeleC::h_prob_parm_device->d_pmf_X = PeleC::prob_parm_host->pmf_X.data();
  PeleC::h_prob_parm_device->d_pmf_Y = PeleC::prob_parm_host->pmf_Y.data();
  PeleC::d_prob_parm_device->d_pmf_X = PeleC::prob_parm_host->pmf_X.data();
  PeleC::d_prob_parm_device->d_pmf_Y = PeleC::prob_parm_host->pmf_Y.data();
}

// Map species name -> index for your 14-spec mechanism
static int species_id_from_name(const std::string& s)
{
    if (s == "H2")   return 0;
    if (s == "H")    return 1;
    if (s == "AR")   return 2;
    if (s == "N2")   return 3;
    if (s == "HE")   return 4;
    if (s == "O")    return 5;
    if (s == "OH")   return 6;
    if (s == "HCO")  return 7;
    if (s == "HO2")  return 8;
    if (s == "H2O")  return 9;
    if (s == "CO")   return 10;
    if (s == "O2")   return 11;
    if (s == "H2O2") return 12;
    if (s == "CO2")  return 13;
    return 0;
}

extern "C" {
void amrex_probinit(const int*, const int*, const int*,
                    const amrex::Real* problo,
                    const amrex::Real* probhi)
{
    auto* P = PeleC::h_prob_parm_device;
    auto eos = pele::physics::PhysicsType::eos();

    // -------------------------------------------------
    //  ONE-SIDED DETONATION LAUNCH (hotspot on left)
    // -------------------------------------------------
    
    // DEFAULT VALUES (can be overridden by input file)
    P->idir = 0;          // x-direction split (default)
    P->frac = 0.02;       // hotspot occupies 2% of domain width

    // --- Left = hotspot ---
    P->rho_l = 1.2;        // kg/m^3
    P->T_l   = 3000.0;     // K (ignite)
    P->p_l   = 1.0e5;      // Pa (same as ambient or slightly higher)
    P->u_l   = 0.0;

    // --- Right = fresh mixture ---
    P->rho_r = 1.2;        // kg/m^3
    P->T_r   = 300.0;      // K
    P->p_r   = 1.0e5;      // Pa
    P->u_r   = 0.0;

    // --- Read from input file (OVERRIDE defaults) ---
    amrex::ParmParse pp("prob");
    
    pp.query("idir", P->idir);        // ADD THIS LINE
    pp.query("frac", P->frac);        // ADD THIS LINE
    pp.query("rho_l", P->rho_l);      // ADD THIS LINE
    pp.query("T_l", P->T_l);          // ADD THIS LINE
    pp.query("p_l", P->p_l);          // ADD THIS LINE
    pp.query("u_l", P->u_l);          // ADD THIS LINE
    pp.query("rho_r", P->rho_r);      // ADD THIS LINE
    pp.query("T_r", P->T_r);          // ADD THIS LINE
    pp.query("p_r", P->p_r);          // ADD THIS LINE
    pp.query("u_r", P->u_r);          // ADD THIS LINE

    // --- Gases ---
    std::string leftGas  = "H2";
    std::string rightGas = "O2";
    pp.query("left_gas", leftGas);
    pp.query("right_gas", rightGas);

    P->left_gas_id  = species_id_from_name(leftGas);
    P->right_gas_id = species_id_from_name(rightGas);

    // --- Validate idir ---
    if (P->idir < 0 || P->idir >= AMREX_SPACEDIM) {
        amrex::Abort("invalid idir !!!");
    }

    // --- Split location ---
    for (int d = 0; d < AMREX_SPACEDIM; ++d)
        P->split[d] = problo[d] + P->frac * (probhi[d] - problo[d]);

    // --- Compute rho*e for both states ---
    amrex::Real e = 0.0;
    amrex::Real Yl[NUM_SPECIES] = {0.0};
    if (P->left_gas_id >= 0 && P->left_gas_id < NUM_SPECIES)
        Yl[P->left_gas_id] = 1.0;
    eos.RYP2E(P->rho_l, Yl, P->p_l, e);
    P->rhoe_l = P->rho_l * e;

    amrex::Real Yr[NUM_SPECIES] = {0.0};
    if (P->right_gas_id >= 0 && P->right_gas_id < NUM_SPECIES)
        Yr[P->right_gas_id] = 1.0;
    eos.RYP2E(P->rho_r, Yr, P->p_r, e);
    P->rhoe_r = P->rho_r * e;
}
} // extern "C"


// Optional hooks
void PeleC::problem_post_init() {}
void PeleC::problem_post_timestep() {}
void PeleC::problem_post_restart() {}
