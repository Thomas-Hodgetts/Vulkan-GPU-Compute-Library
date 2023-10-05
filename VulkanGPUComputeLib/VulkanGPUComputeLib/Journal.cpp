#include "Journal.h"

void Journal::AddWarningEntry(std::wstring fileName, std::wstring warningCode, std::wstring warningMessage)
{
	std::wstring emptyFileLocation = L"";
	nlohmann::json journal = OpenFile(emptyFileLocation, fileName);
	std::string warnCode = WideStringToString(warningCode);
	std::string msg = WideStringToString(warningMessage);
	journal["Warnings"][warnCode].push_back(msg);
	CopyToFile(journal, emptyFileLocation, fileName);
}

void Journal::AddWarningEntry(std::wstring fileLocation, std::wstring fileName, std::wstring warningCode, std::wstring warningMessage)
{
	nlohmann::json journal = OpenFile(fileLocation, fileName);
	std::string warnCode = WideStringToString(warningCode);
	std::string msg = WideStringToString(warningMessage);
	journal["Warnings"][warnCode].push_back(msg);
	CopyToFile(journal, fileLocation, fileName);
}

void Journal::AddWarningEntry(std::string fileName, std::string warningCode, std::string warningMessage)
{
	std::wstring emptyFileLocation = L"";
	std::wstring src = StringToWideString(fileName);
	nlohmann::json journal = OpenFile(emptyFileLocation, src);
	journal["Warnings"][warningCode].push_back(warningMessage);
	CopyToFile(journal, emptyFileLocation, src);
}

void Journal::AddWarningEntry(std::string fileLocation, std::string fileName, std::string warningCode, std::string warningMessage)
{
	std::wstring src = StringToWideString(fileName);
	std::wstring folder = StringToWideString(fileLocation);
	nlohmann::json journal = OpenFile(folder, src);
	journal["Warnings"][warningCode].push_back(warningMessage);
	CopyToFile(journal, folder, src);
}

void Journal::AddErrorEntry(std::wstring fileName, std::wstring errorCode, std::wstring errorMessage)
{
	std::wstring emptyFileLocation = L"";
	nlohmann::json journal = OpenFile(emptyFileLocation, fileName);
	std::string errCode = WideStringToString(errorCode);
	std::string msg = WideStringToString(errorMessage);
	journal["Errors"][errCode].push_back(msg);
	CopyToFile(journal, emptyFileLocation, fileName);
}

void Journal::AddErrorEntry(std::wstring fileLocation, std::wstring fileName, std::wstring errorCode, std::wstring errorMessage)
{
	nlohmann::json journal = OpenFile(fileLocation, fileName);
	std::string errCode = WideStringToString(errorCode);
	std::string msg = WideStringToString(errorMessage);
	journal["Errors"][errCode].push_back(msg);
	CopyToFile(journal, fileLocation, fileName);
}

void Journal::AddErrorEntry(std::string fileName, std::string errorCode, std::string errorMessage)
{
	std::wstring emptyFileLocation = L"";
	std::wstring src = StringToWideString(fileName);
	nlohmann::json journal = OpenFile(emptyFileLocation, src);
	journal["Errors"][errorCode].push_back(errorMessage);
	CopyToFile(journal, emptyFileLocation, src);
}

void Journal::AddErrorEntry(std::string fileLocation, std::string fileName, std::string errorCode, std::string errorMessage)
{
	std::wstring src = StringToWideString(fileName);
	std::wstring srcFolder = StringToWideString(fileLocation);
	nlohmann::json journal = OpenFile(srcFolder, src);
	journal["Errors"][errorCode].push_back(errorMessage);
	CopyToFile(journal, srcFolder, src);
}

void Journal::AddMessage(std::wstring fileName, std::wstring msg)
{
	std::wstring emptyFileLocation = L"";
	nlohmann::json journal = OpenFile(emptyFileLocation, fileName);
	std::string newMessage = WideStringToString(msg);
	journal["Message"].push_back(newMessage);
	CopyToFile(journal, emptyFileLocation, fileName);
}

void Journal::AddMessage(std::wstring fileLocation, std::wstring fileName, std::wstring msg)
{
	nlohmann::json journal = OpenFile(fileLocation, fileName);
	std::string newMessage = WideStringToString(msg);
	journal["Message"].push_back(newMessage);
	CopyToFile(journal, fileLocation, fileName);
}

void Journal::AddMessage(std::string fileName, std::string msg)
{
	std::wstring emptyFileLocation = L"";
	std::wstring src = StringToWideString(fileName);
	nlohmann::json journal = OpenFile(emptyFileLocation, src);
	journal["Message"].push_back(msg);
	CopyToFile(journal, emptyFileLocation, src);
}

void Journal::AddMessage(std::string fileLocation, std::string fileName, std::string msg)
{
	std::wstring src = StringToWideString(fileName);
	std::wstring srcFolder = StringToWideString(fileLocation);
	nlohmann::json journal = OpenFile(srcFolder, src);
	journal["Message"].push_back(msg);
	CopyToFile(journal, srcFolder, src);
}

void Journal::AddPerformaceEntry(std::wstring fileName, std::wstring testName, int testNumber, double timeTaken)
{
	std::wstring emptyFileLocation = L"";
	std::string testID = WideStringToString(testName);
	nlohmann::json journal = OpenFile(emptyFileLocation, fileName);
	journal["Testing"][testID][testNumber]["Total Time"] = timeTaken;
	CopyToFile(journal, emptyFileLocation, fileName);

}

void Journal::AddPerformaceEntry(std::wstring fileLocation, std::wstring fileName, std::wstring testName, int testNumber, double timeTaken)
{
	std::string testID = WideStringToString(testName);
	nlohmann::json journal = OpenFile(fileLocation, fileName);
	journal["Testing"][testID][testNumber]["Total Time"] = timeTaken;
	CopyToFile(journal, fileLocation, fileName);
}

//void Journal::AddPerformaceEntry(std::wstring fileName, std::wstring testName, int testNumber, std::chrono::steady_clock::time_point* beginTime, std::chrono::steady_clock::time_point* endTime)
//{
//	std::wstring emptyFileLocation = L"";
//	std::string testID = WideStringToString(testName);
//	nlohmann::json journal = OpenFile(emptyFileLocation, fileName);
//
//
//	if (beginTime == nullptr)
//	{
//		if (endTime != nullptr)
//		{
//			std::chrono::milliseconds startTime = std::chrono::time_point_cast<std::chrono::milliseconds>(*beginTime).time_since_epoch();
//			journal["Testing"][testID][testNumber]["End Time"] = std::chrono::duration<double>(startTime).count();
//
//			double start = journal["Testing"][testID][testNumber]["Start Time"];
//			double end = journal["Testing"][testID][testNumber]["End Time"];
//
//			journal["Testing"][testID][testNumber]["Total Time"] = (end - start);
//		}
//		else
//		{
//			return;
//		}
//	}
//	else
//	{
//		std::chrono::milliseconds startTime = std::chrono::time_point_cast<std::chrono::milliseconds>(*beginTime).time_since_epoch();
//		journal["Testing"][testID][testNumber]["Start Time"] = std::chrono::duration<double>(startTime).count();
//	}
//}
//
//void Journal::AddPerformaceEntry(std::wstring fileName, std::wstring fileLocation, std::wstring testName, int testNumber, std::chrono::steady_clock::time_point* beginTime, std::chrono::steady_clock::time_point* endTime)
//{
//	std::string testID = WideStringToString(testName);
//	nlohmann::json journal = OpenFile(fileLocation, fileName);
//
//	if (beginTime == nullptr)
//	{
//		if (endTime != nullptr)
//		{
//			std::chrono::milliseconds startTime = std::chrono::time_point_cast<std::chrono::milliseconds>(*beginTime).time_since_epoch();
//			journal["Testing"][testID][testNumber]["End Time"] = std::chrono::duration<double>(startTime).count();
//
//			double start = journal["Testing"][testID][testNumber]["Start Time"];
//			double end = journal["Testing"][testID][testNumber]["End Time"];
//
//			journal["Testing"][testID][testNumber]["Total Time"] = (end - start);
//		}
//		else
//		{
//			return;
//		}
//	}
//	else
//	{
//		std::chrono::milliseconds startTime = std::chrono::time_point_cast<std::chrono::milliseconds>(*beginTime).time_since_epoch();
//		journal["Testing"][testID][testNumber]["Start Time"] = std::chrono::duration<double>(startTime).count();
//	}	
//
//}
//
//void Journal::AddPerformaceEntry(std::string fileName, std::string testName, int testNumber, std::chrono::steady_clock::time_point* beginTime, std::chrono::steady_clock::time_point* endTime)
//{
//	std::wstring emptyFileLocation = L"";
//	std::wstring file = StringToWideString(fileName);
//	nlohmann::json journal = OpenFile(emptyFileLocation, file);
//
//	if (beginTime == nullptr)
//	{
//		if (endTime != nullptr)
//		{
//			std::chrono::milliseconds startTime = std::chrono::time_point_cast<std::chrono::milliseconds>(*beginTime).time_since_epoch();
//			journal["Testing"][testName][testNumber]["End Time"] = std::chrono::duration<double>(startTime).count();
//
//			double start = journal["Testing"][testName][testNumber]["Start Time"];
//			double end = journal["Testing"][testName][testNumber]["End Time"];
//
//			journal["Testing"][testName][testNumber]["Total Time"] = (end - start);
//		}
//		else
//		{
//			return;
//		}
//	}
//	else
//	{
//		std::chrono::milliseconds startTime = std::chrono::time_point_cast<std::chrono::milliseconds>(*beginTime).time_since_epoch();
//		journal["Testing"][testName][testNumber]["Start Time"] = std::chrono::duration<double>(startTime).count();
//	}
//}
//
//void Journal::AddPerformaceEntry(std::string fileName, std::string fileLocation, std::string testName, int testNumber, std::chrono::steady_clock::time_point* beginTime, std::chrono::steady_clock::time_point* endTime)
//{
//	std::wstring location = StringToWideString(fileLocation);
//	std::wstring file = StringToWideString(fileName);
//	nlohmann::json journal = OpenFile(location, file);
//
//	if (beginTime == nullptr)
//	{
//		if (endTime != nullptr)
//		{
//			std::chrono::milliseconds startTime = std::chrono::time_point_cast<std::chrono::milliseconds>(*beginTime).time_since_epoch();
//			journal["Testing"][testName][testNumber]["End Time"] = std::chrono::duration<double>(startTime).count();
//
//			double start = journal["Testing"][testName][testNumber]["Start Time"];
//			double end = journal["Testing"][testName][testNumber]["End Time"];
//
//			journal["Testing"][testName][testNumber]["Total Time"] = (end - start);
//		}
//		else
//		{
//			return;
//		}
//	}
//	else
//	{
//		std::chrono::milliseconds startTime = std::chrono::time_point_cast<std::chrono::milliseconds>(*beginTime).time_since_epoch();
//		journal["Testing"][testName][testNumber]["Start Time"] = std::chrono::duration<double>(startTime).count();
//	}
//}

void Journal::CopyToFile(nlohmann::json& outData, std::wstring fileLocation, std::wstring fileName)
{
	std::filesystem::path pathway;
	if (fileLocation.size() > 0)
	{
		pathway.append(fileLocation);
		pathway.append(L"/");
		pathway.append(fileName);
	}
	else
	{
		pathway = std::filesystem::current_path();
		pathway.append(fileName);
	}
	
	std::ofstream dst;
	dst.open(pathway, std::ios_base::out);
	dst << outData;
}

nlohmann::json Journal::CreateNewJournalFile(std::filesystem::path& path)
{
	std::ofstream ofs(path);
	nlohmann::json inputFile;
	ofs << inputFile;
	ofs.close();
	return inputFile;
}

nlohmann::json Journal::OpenFile(std::wstring& fileLocation, std::wstring& fileName)
{
	std::filesystem::path pathway;
	if (fileLocation.size() > 0)
	{
		pathway.append(fileLocation);
		pathway.append(L"/");
		pathway.append(fileName);
		pathway.append(L".json");
	}
	else
	{
		pathway = std::filesystem::current_path();
		fileName.append(L".json");
		pathway.append(fileName);
	}
	bool doesFileExist = std::filesystem::exists(pathway);
	if (doesFileExist)
	{
		std::ifstream inFile(pathway);
		nlohmann::json inputFile;
		if (inFile.is_open())
		{

			inFile >> inputFile;
			return inputFile;
		}
		else
		{
			return inputFile;
		}
	}
	else
	{
		return CreateNewJournalFile(pathway);
	}
}
