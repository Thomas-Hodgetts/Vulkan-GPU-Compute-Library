struct CalculatorShader
{
    float m_FirstNumber, m_SecondNumber, m_Output, m_Padding;
    int m_Addition, m_Subtraction, m_Multiplication, m_Division;
};

[[vk::binding(0, 0)]] RWStructuredBuffer<CalculatorShader> calculatorData;

[numthreads(1, 1, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
    bool calculatorMode = calculatorData[DTid.x].m_Addition;
    calculatorData[DTid.x].m_Addition = calculatorMode;
    if (calculatorData[DTid.x].m_Addition > 0)
    {
        calculatorData[DTid.x].m_Output = calculatorData[DTid.x].m_FirstNumber + calculatorData[DTid.x].m_SecondNumber;
    }
    else if (calculatorData[DTid.x].m_Subtraction > 0)
    {
        calculatorData[DTid.x].m_Output = calculatorData[DTid.x].m_FirstNumber - calculatorData[DTid.x].m_SecondNumber;
    }
    else if (calculatorData[DTid.x].m_Multiplication > 0)
    {
        calculatorData[DTid.x].m_Output = calculatorData[DTid.x].m_FirstNumber * calculatorData[DTid.x].m_SecondNumber;
    }
    else if (calculatorData[DTid.x].m_Division > 0)
    {
        calculatorData[DTid.x].m_Output = calculatorData[DTid.x].m_FirstNumber / calculatorData[DTid.x].m_SecondNumber;
    }
    
}