#include "CalculatorApp.h"

void GenerateAndRunShaderCompile()
{

	std::filesystem::path basePath = std::filesystem::current_path();

	//create new shader compile file
	std::ofstream outFile(basePath.string() + "/ShaderCompile.bat");

	//sort which files need to be compiled
	for (const std::filesystem::directory_entry& entry : std::filesystem::directory_iterator(basePath))
	{
		char fileExtension[4];
		std::string filePath = entry.path().string();
		filePath.copy(fileExtension, 4, filePath.size() - 4);
		//std::cout << fileExtension << std::endl;
		if (fileExtension[0] == 'h' && fileExtension[1] == 'l' && fileExtension[2] == 's' && fileExtension[3] == 'l')
		{
			//Generate compile command
			std::string newName = filePath;
			newName[newName.size() - 4] = 's';
			newName[newName.size() - 3] = 'p';
			newName[newName.size() - 2] = 'v';
			newName.pop_back();

			std::string outputText = "C:/VulkanSDK/1.3.246.1/Bin/dxc.exe -spirv -T cs_6_0 -E main " + filePath + " - Fo " + newName;
			outFile << outputText << std::endl;
			int arrSize = filePath.size() - 1;
		}
	}
	//outFile << "pause" << std::endl;

	//close and run the file
	outFile.close();
	system("ShaderCompile.bat");
}


int main()
{
	//GenerateAndRunShaderCompile();

	CalculatorApp();
	

}