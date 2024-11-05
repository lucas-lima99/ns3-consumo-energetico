/*
 * This script simulates a simple network to explain how the Lora energy model
 * works.
 */

#include "ns3/end-device-lora-phy.h"
#include "ns3/gateway-lora-phy.h"
#include "ns3/end-device-lora-mac.h"
#include "ns3/gateway-lora-mac.h"
#include "ns3/simulator.h"
#include "ns3/log.h"
#include "ns3/pointer.h"
#include "ns3/constant-position-mobility-model.h"
#include "ns3/lora-helper.h"
#include "ns3/mobility-helper.h"
#include "ns3/node-container.h"
#include "ns3/position-allocator.h"
#include "ns3/periodic-sender-helper.h"
#include "ns3/command-line.h"
#include "ns3/basic-energy-source-helper.h"
#include "ns3/lora-radio-energy-model-helper.h"
#include "ns3/end-device-lora-mac.h"
#include "ns3/file-helper.h"
#include "ns3/names.h"
#include <algorithm>
#include <ctime>
#include <time.h>
#include <math.h>
#include "ns3/lora-tx-current-model.h"
#include "ns3/network-server-helper.h"
#include "ns3/correlated-shadowing-propagation-loss-model.h"
#include "ns3/building-penetration-loss.h"
#include "ns3/building-allocator.h"
#include "ns3/buildings-helper.h"
#include "ns3/forwarder-helper.h"
#include "ns3/random-variable-stream.h"
#include "ns3/propagation-loss-model.h"
#include <fstream>
#include <iostream>
#include <string>
#include <stack>
#include "ns3/double.h"
#include <cstdlib>
#include <map>



using namespace ns3;
using namespace lorawan;

NS_LOG_COMPONENT_DEFINE ("LoraEnergyModelExample");



// Test if the file is empty
bool is_empty(std::ifstream& pFile)
{
	return pFile.peek() == std::ifstream::traits_type::eof();
}


void PrintPositions(NodeContainer nodes, std::string filename, int algoritmo)

{
	std::ifstream inFile(filename.c_str());
	bool fileIsEmpty = is_empty(inFile);
	inFile.close();

	std::ofstream outFile;

	if (fileIsEmpty){
		outFile.open(filename.c_str(), std::ios::out); //abre em modo de escrita
		outFile << "Nó,PosX,PosY,PosZ,Algoritmo" << std::endl;

	}
	outFile.open(filename.c_str(), std::ios::app); // abre em modo de anexação
	for (NodeContainer::Iterator it = nodes.Begin(); it != nodes.End(); ++it) {
	        Ptr<Node> node = *it;
	        Ptr<MobilityModel> mobility = node->GetObject<MobilityModel>();
	        Vector pos = mobility->GetPosition();
	//        outFile << "Node " << node->GetId() << ": x=" << pos.x << ", y=" << pos.y << ", z=" << pos.z << std::endl;
	        outFile << node->GetId()<< "," << pos.x << "," << pos.y << "," << pos.z << "," << algoritmo << std::endl;
	}
//	outFile << "--" << std::endl;
	outFile.close();
}

// To be used in tic toc time counter
clock_t startTimer;
time_t beginTimer;
//
// Implementation of tic, i.e., start time counter
void
tic()
{
	beginTimer = time(&beginTimer);
	struct tm * timeinfo;
	timeinfo = localtime(&beginTimer);
	std::cout << "simulation start at: " << asctime(timeinfo) << std::endl;
}
// implementation of toc, i.e., stop time counter
double
toc()
{
	time_t finishTimer = time(&finishTimer);
	double simTime = difftime(finishTimer, beginTimer) / 60.0;
	struct tm * timeinfo;
	timeinfo = localtime(&finishTimer);
	std::cout << "simulation finished at: " << asctime(timeinfo) << std::endl;
	//
	std::cout << "Time elapsed: " << simTime << " minutes" << std::endl;
	//
	return simTime;
}


int main (int argc, char *argv[])
{
	tic();
  // Set up logging
  LogComponentEnable ("LoraEnergyModelExample", LOG_LEVEL_ALL);
//  LogComponentEnable ("LoraRadioEnergyModel", LOG_LEVEL_ALL);
//   LogComponentEnable ("LoraChannel", LOG_LEVEL_INFO);
//   LogComponentEnable ("LoraPhy", LOG_LEVEL_ALL);
//   LogComponentEnable ("EndDeviceLoraPhy", LOG_LEVEL_ALL);
//   LogComponentEnable ("GatewayLoraPhy", LOG_LEVEL_ALL);
//   LogComponentEnable ("LoraInterferenceHelper", LOG_LEVEL_ALL);
//   LogComponentEnable ("LoraMac", LOG_LEVEL_ALL);
//   LogComponentEnable ("EndDeviceLoraMac", LOG_LEVEL_ALL);
//   LogComponentEnable ("GatewayLoraMac", LOG_LEVEL_ALL);
//   LogComponentEnable ("LogicalLoraChannelHelper", LOG_LEVEL_ALL);
//   LogComponentEnable ("LogicalLoraChannel", LOG_LEVEL_ALL);
//   LogComponentEnable ("LoraHelper", LOG_LEVEL_ALL);
//   LogComponentEnable ("LoraPhyHelper", LOG_LEVEL_ALL);
//   LogComponentEnable ("LoraMacHelper", LOG_LEVEL_ALL);
//   LogComponentEnable ("OneShotSenderHelper", LOG_LEVEL_ALL);
//   LogComponentEnable ("OneShotSender", LOG_LEVEL_ALL);
//   LogComponentEnable ("LoraMacHeader", LOG_LEVEL_ALL);
//   LogComponentEnable ("LoraFrameHeader", LOG_LEVEL_ALL);
  LogComponentEnableAll (LOG_PREFIX_FUNC);
  LogComponentEnableAll (LOG_PREFIX_NODE);
  LogComponentEnableAll (LOG_PREFIX_TIME);



  int radius = 2000;
  int nDevices = 100;
  int algoritmo = 6;
  int targetRealocation = 0;
  double batteryEnergyFinal;
  double batteryEnergyInit = 10000;
  double batteryVoltage = 3.3;
  double appPeriodsSeconds = 600;
  bool realisticChannelModel = false;
  int seed=1;
  uint32_t runSeed=1;
  bool fixedSeed = false;
  bool print = false;
  int packetsize = 23;
  double N = 3.78;
  double sigma = 0;
  int txPowerdBm = 12;
  int hours = 2;
  double distanceReference = 8.1;

	if (fixedSeed){
		RngSeedManager::SetSeed(seed);
		RngSeedManager::SetRun(runSeed);
	}

	//Getting seed and runSeed for checking and displaying purposes
	seed = RngSeedManager::GetSeed();
	runSeed = RngSeedManager::GetRun();

  std::string outputDir = "./";
  std::string filename = "DadosBattery";
  std::string chFilename;



	CommandLine cmd;
	cmd.AddValue ("nDevices",
			"Number of end devices to include in the simulation",
			nDevices);
	cmd.AddValue ("radius",
			"The radius of the area to simulate",
			radius);
	cmd.AddValue ("appPeriodsSeconds",
			"The period in seconds to be used by periodically transmitting applications",
			appPeriodsSeconds);
	cmd.AddValue ("outputDir",
				"Output directory",
				outputDir);
	cmd.AddValue ("filename",
					"Output file name",
					filename);
	cmd.AddValue ("algoritmo",
			"Algoritmo de alocaçao de SF a ser utilizado",
			algoritmo);
	cmd.AddValue ("packetsize",
				"Tamanho do pacote em bytes",
				packetsize);
	cmd.AddValue ("runSeed",
				"Set runseed",
				runSeed);
	cmd.AddValue ("expoenteN",
				  "expoente N da regressão para modelo los ou nlos",
				  N);
	cmd.AddValue ("distanceReference",
				 " distancia de referencia de regressão para modelo los ou nlos",
				  distanceReference);
	cmd.AddValue ("sigma",
				  "Desvio padrão do PL em dB",
				  sigma);
	cmd.AddValue ("txPowerdBm",
				  "Potência de transmissão em dBm",
				  txPowerdBm);
	cmd.AddValue ("hours",
				  "Tempo de simulação em horas",
				  hours);
	cmd.Parse (argc, argv);



// Displaying the seed and runSeed being used in the simulation
	std::cout << "Seed: " << seed << ", RunSeed: " << runSeed << std::endl;

  chFilename = outputDir+"/"+ filename+".txt";
  std::ifstream file(chFilename.c_str());
  bool firstTime = is_empty(file);

  file.close();
  std::ofstream myfile;


  myfile.open(chFilename.c_str(), std::ofstream::app);
  if ( firstTime )
  	{
  		// file is empty: write down the header
  		myfile <<
//  				"Amplificador," <<
  				"RunSeed," <<
				"Seed," <<
				"packetSize," <<
				"N," <<
				"d," <<
				"sigma," <<
  				"Algoritmo," <<
  				"Radius," <<
  				"nDevices," <<
  				"appPeriodsSeconds," <<
				"batteryVoltage," <<
				"batteryEnergyInit," <<
				"hours," <<
				"TxPowerdBm," <<
				"PHYTotal," <<
				"PHYSuccessful," <<
				"PHYInterfered," <<
				"PHYNoMoreReceivers," <<
				"PHYUnderSensitivity," <<
				"PHYLostBecauseTX," <<
				"batteryEnergyFinal";
  	}

  	myfile << std::endl <<
  			runSeed << "," <<
			seed << "," <<
  			packetsize << "," <<
			N << "," <<
			distanceReference << "," <<
			sigma << "," <<
  			algoritmo << "," <<
  			radius<< "," <<
  			nDevices << "," <<
  			appPeriodsSeconds << "," <<
  			batteryVoltage << "," <<
			batteryEnergyInit << "," <<
			hours << "," <<
  			txPowerdBm << ",";

  	myfile.close ();



  /************************
  *  Create the channel  *
  ************************/

  NS_LOG_INFO ("Creating the channel...");


  Ptr<NormalRandomVariable> gaussianVar = CreateObject<NormalRandomVariable> ();
  gaussianVar->SetAttribute ("Mean", DoubleValue (0.0));
  gaussianVar->SetAttribute ("Variance", DoubleValue (sigma * sigma));
  double gaussianValue = gaussianVar->GetValue();

  NS_LOG_INFO("valor gaussiano:  " << gaussianValue << " gaussiano");


//  // Create the lora channel object // Caso copie esse código, use esse canal. O que não está comentado é aquele que leva em consideração o shadowing. Aí você pode desconsiderar as linhas 292 até 295 também
//  Ptr<LogDistancePropagationLossModel> loss = CreateObject<LogDistancePropagationLossModel> ();
//  loss->SetPathLossExponent (d);
//  loss->SetReference (1, 7.7);

  // Create the lora channel object

  Ptr<LogDistanceGaussianDistributionPropagationLossModel> loss = CreateObject<LogDistanceGaussianDistributionPropagationLossModel> ();
  loss->SetPathLossExponent (N);
  loss->SetReference (1, distanceReference);
  loss->SetGaussianVariable(gaussianValue);
  NS_LOG_INFO("Gaussian na classe  " << loss->GetGaussianVariable() << " gaussiano");

  if (realisticChannelModel)
  	{
  		// Create the correlated shadowing component
  		Ptr<CorrelatedShadowingPropagationLossModel> shadowing = CreateObject<CorrelatedShadowingPropagationLossModel> ();

  		// Aggregate shadowing to the logdistance loss
  		loss->SetNext (shadowing);

  		// Add the effect to the channel propagation loss
  		Ptr<BuildingPenetrationLoss> buildingLoss = CreateObject<BuildingPenetrationLoss> ();

  		shadowing->SetNext (buildingLoss);
  	}
  Ptr<PropagationDelayModel> delay = CreateObject<ConstantSpeedPropagationDelayModel> ();

  Ptr<LoraChannel> channel = CreateObject<LoraChannel> (loss, delay);

  /************************
  *  Create the helpers  *
  ************************/

  NS_LOG_INFO ("Setting up helpers...");

  MobilityHelper mobility;
  mobility.SetPositionAllocator ("ns3::UniformDiscPositionAllocator",
  			"rho", DoubleValue (radius),
  			"X", DoubleValue (0.0),
  			"Y", DoubleValue (0.0));
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
//  Ptr<ListPositionAllocator> allocator = CreateObject<ListPositionAllocator> ();
//  allocator->Add (Vector (1000,0,0));
//  allocator->Add (Vector (0,0,0));
//  mobility.SetPositionAllocator (allocator);
//  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");

  // Create the LoraPhyHelper
  LoraPhyHelper phyHelper = LoraPhyHelper ();
  phyHelper.SetChannel (channel);

  // Create the LoraMacHelper
  LoraMacHelper macHelper = LoraMacHelper ();

  // Create the LoraHelper
  LoraHelper helper = LoraHelper ();
  helper.EnablePacketTracking (chFilename); // Output filename

  /************************
  *  Create End Devices  *
  ************************/

  NS_LOG_INFO ("Creating the end device...");

  // Create a set of nodes
  NodeContainer endDevices;
  endDevices.Create (nDevices);

  // Assign a mobility model to the node
  mobility.Install (endDevices);

  // Create the LoraNetDevices of the end devices
  phyHelper.SetDeviceType (LoraPhyHelper::ED);
  macHelper.SetDeviceType (LoraMacHelper::ED);



  NetDeviceContainer endDevicesNetDevices = helper.Install (phyHelper, macHelper, endDevices);

  /*********************
   *  Create Gateways  *
   *********************/

  NS_LOG_INFO ("Creating the gateway...");
  NodeContainer gateways;
  gateways.Create (1);

  Ptr<ListPositionAllocator> allocator = CreateObject<ListPositionAllocator> ();//ADRcode
  	// Make it so that nodes are at a certain height > 0 //ADRcode
  allocator->Add (Vector (0.0, 0.0, 15.0)); //ADRcode

  mobility.SetPositionAllocator (allocator);
  mobility.Install (gateways);

  // Create a netdevice for each gateway
  phyHelper.SetDeviceType (LoraPhyHelper::GW);
  macHelper.SetDeviceType (LoraMacHelper::GW);
  helper.Install (phyHelper, macHelper, gateways);


	/**********************
	 *  Handle buildings  *
	 **********************/

	double xLength = 130;
	double deltaX = 32;
	double yLength = 64;
	double deltaY = 17;
	int gridWidth = 2 * radius / (xLength + deltaX);
	int gridHeight = 2 * radius / (yLength + deltaY);
	if (realisticChannelModel == false)
	{
		gridWidth = 0;
		gridHeight = 0;
	}
	Ptr<GridBuildingAllocator> gridBuildingAllocator;
	gridBuildingAllocator = CreateObject<GridBuildingAllocator> ();
	gridBuildingAllocator->SetAttribute ("GridWidth", UintegerValue (gridWidth));
	gridBuildingAllocator->SetAttribute ("LengthX", DoubleValue (xLength));
	gridBuildingAllocator->SetAttribute ("LengthY", DoubleValue (yLength));
	gridBuildingAllocator->SetAttribute ("DeltaX", DoubleValue (deltaX));
	gridBuildingAllocator->SetAttribute ("DeltaY", DoubleValue (deltaY));
	gridBuildingAllocator->SetAttribute ("Height", DoubleValue (6));
	gridBuildingAllocator->SetBuildingAttribute ("NRoomsX", UintegerValue (2));
	gridBuildingAllocator->SetBuildingAttribute ("NRoomsY", UintegerValue (4));
	gridBuildingAllocator->SetBuildingAttribute ("NFloors", UintegerValue (2));
	gridBuildingAllocator->SetAttribute ("MinX", DoubleValue (-gridWidth * (xLength + deltaX) / 2 + deltaX / 2));
	gridBuildingAllocator->SetAttribute ("MinY", DoubleValue (-gridHeight * (yLength + deltaY) / 2 + deltaY / 2));
	BuildingContainer bContainer = gridBuildingAllocator->Create (gridWidth * gridHeight);

	BuildingsHelper::Install (endDevices);
	BuildingsHelper::Install (gateways);
	BuildingsHelper::MakeMobilityModelConsistent ();

	// Print the buildings
	if (print)
	{
		std::ofstream myfile;
		myfile.open ("buildings.txt");
		std::vector<Ptr<Building> >::const_iterator it;
		int j = 1;
		for (it = bContainer.Begin (); it != bContainer.End (); ++it, ++j)
		{
			Box boundaries = (*it)->GetBoundaries ();
			myfile << "set object " << j << " rect from " << boundaries.xMin << "," << boundaries.yMin << " to " << boundaries.xMax << "," << boundaries.yMax << std::endl;
		}
		myfile.close ();

	}


  macHelper.SetSpreadingFactorsUp (endDevices, gateways, channel, algoritmo);

  for (uint32_t i = 0; i < endDevices.GetN(); ++i) {
      Ptr<Node> node = endDevices.Get(i);
      for (uint32_t j = 0; j < node->GetNDevices(); ++j) {
          Ptr<LoraNetDevice> loraNetDevice = DynamicCast<LoraNetDevice>(node->GetDevice(j));

          if (loraNetDevice) {
              Ptr<EndDeviceLoraMac> mac = loraNetDevice->GetMac()->GetObject<EndDeviceLoraMac>();
              if (mac) {
                  mac->SetTransmissionPower(txPowerdBm);
                  std::cout << "Potencia de transmissao para o dispositivo" << i << ": " << txPowerdBm << " dBm\n";

              } else {
            	  std::cerr << "Erro: MAC nao encontrado para o dispositivo " << i << "\n";
                  }
          }
      }
  }


  /*********************************************
   *  Install applications on the end devices  *
   *********************************************/

  PeriodicSenderHelper periodicSenderHelper;
  periodicSenderHelper.SetPeriod (Seconds (appPeriodsSeconds));
    periodicSenderHelper.SetPacketSize (packetsize);

  periodicSenderHelper.Install (endDevices);

  /************************
   * Install Energy Model *
   ************************/

  BasicEnergySourceHelper basicSourceHelper;
  LoraRadioEnergyModelHelper radioEnergyHelper;

  // configure energy source
  basicSourceHelper.Set ("BasicEnergySourceInitialEnergyJ", DoubleValue (batteryEnergyInit)); // Energy in J
  basicSourceHelper.Set ("BasicEnergySupplyVoltageV", DoubleValue (batteryVoltage)); //Voltage in V

  // correntes padrão exemplo energy-model
//  radioEnergyHelper.Set ("StandbyCurrentA", DoubleValue (0.0014));
//  radioEnergyHelper.Set ("TxCurrentA", DoubleValue (0.028));
//  radioEnergyHelper.Set ("SleepCurrentA", DoubleValue (0.0000015));
//  radioEnergyHelper.Set ("RxCurrentA", DoubleValue (0.0112));
//  radioEnergyHelper.SetTxCurrentModel ("ns3::ConstantLoraTxCurrentModel",
//                                       "TxCurrent", DoubleValue (0.028));

//  radioEnergyHelper.Set ("StandbyCurrentA", DoubleValue (0.0014));
//  radioEnergyHelper.Set ("TxCurrentA", DoubleValue (0.082));
//  radioEnergyHelper.Set ("SleepCurrentA", DoubleValue (0.000001));
//  radioEnergyHelper.Set ("RxCurrentA", DoubleValue (0.0112));
//  radioEnergyHelper.Set ("IdleCurrentA", DoubleValue (0.00015));
  radioEnergyHelper.SetTxCurrentModel ("ns3::SX1272LoRaWANCurrentModel",
		  	  	  	  	  	  	  	   "TxPowerToTxCurrent", DoubleValue(txPowerdBm),
								       "UsePaBoost", BooleanValue(true));


  // install source on EDs' nodes
  EnergySourceContainer sources = basicSourceHelper.Install (endDevices);

  // install device model
  DeviceEnergyModelContainer deviceModels = radioEnergyHelper.Install
      (endDevicesNetDevices, sources);

  /**************
   * Get output *
   **************/
//  FileHelper fileHelper;
//  fileHelper.ConfigureFile ("battery-level", FileAggregator::COMMA_SEPARATED);
//  fileHelper.WriteProbe ("ns3::DoubleProbe", "/Names/EnergySource/RemainingEnergy", "Output");
  //fileHelper.ConfigureFile ("battery-level", FileAggregator::SPACE_SEPARATED);
  //fileHelper.WriteProbe ("ns3::DoubleProbe", "/Names/EnergySource/RemainingEnergy", "Output");


  /****************
  *  Simulation  *
  ****************/

  Simulator::Stop (Hours (hours));

  Simulator::Schedule(Seconds(0), &PrintPositions, endDevices, "pos_inicial.txt", algoritmo); //posição inicial
//  Simulator::Schedule(Seconds(appPeriodsSeconds), &PrintPositions, endDevices, "pos_final.txt"); //posição final


  Simulator::Run ();

  double energy = 0;
  for(int i=0; i<nDevices; i++){
	  energy += sources.Get(i)->GetRemainingEnergy();
//	  NS_LOG_INFO("energia restante do dispositivo " << i << " igual a " << sources.Get(i)->GetRemainingEnergy());

  }
  batteryEnergyFinal = energy/nDevices;

  Simulator::Destroy ();

  NS_LOG_INFO ("Computing performance metrics...");
 int transientPeriods = 0;
  Time appPeriod = Seconds(appPeriodsSeconds);

//  helper.PrintPerformance (transientPeriods * appPeriod, Seconds(appPeriodsSeconds));
  helper.PrintPerformance (transientPeriods * appPeriod, Hours(hours));

  toc();
  myfile.open (chFilename.c_str(), std::ofstream::app);
  myfile << batteryEnergyFinal;
  myfile.close ();

  return 0;
}
