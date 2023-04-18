std::string RunFileName(uint32_t runNumber, uint32_t linkNumber, uint32_t fileNumber) {
  std::ostringstream sRunFileName;
  sRunFileName << std::setfill('0')
	       << "RunDatFiles/Run" << std::setw(10) << runNumber
	       << "_Link" << linkNumber
	       << "_File" << std::setw(10) << fileNumber
	       << ".bin";
  return sRunFileName.str();
}

std::string CfgFileName(uint32_t runNumber, uint32_t linkNumber, uint32_t fileNumber) {
  std::ostringstream sRunFileName;
  sRunFileName << std::setfill('0')
	       << "RunDatFiles/SuperRun" << std::setw(10) << runNumber
	       << "_Link" << linkNumber
	       << "_File" << std::setw(10) << fileNumber
	       << ".bin";
  return sRunFileName.str();
}
