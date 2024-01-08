#pragma once
#include "vkGpuHandles.h"
#include <filesystem>
#include <fstream>

struct CalculatorData
{
	float m_FirstNumber = 0, m_SecondNumber = 0, m_Output = 0, m_Padding = 0;
	bool m_Addition = false, m_Subtraction = false, m_Multiplication = false, m_Division = false;
};


void CalculatorApp();