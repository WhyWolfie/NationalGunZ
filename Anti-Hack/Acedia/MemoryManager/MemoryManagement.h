class MemoryManagement{
public:
	MemoryManagement(HMODULE baseAddress, uint32_t addressSize){
#ifndef DEBUG
		InitializeCriticalSection(&this->m_lpCriticalSection);
#endif
		printf ("Setting buffer. Address(%X). Size(%X)\n", baseAddress, addressSize);
		if (baseAddress > 0 && addressSize > 0){
			this->m_hBaseAddress = baseAddress;
			this->m_nAddressSize = addressSize;
			this->m_pMemory = new BYTE[addressSize];
			memcpy(this->m_pMemory, (LPVOID)baseAddress, addressSize);
		}else{
			printf ("Failed to set buffer. Address(%X). Size(%X)\n", baseAddress, addressSize);
		}
	}


	bool MemoryEdited(){
#ifndef DEBUG
		EnterCriticalSection(&this->m_lpCriticalSection);
#endif
		bool bRet = memcmp(this->m_pMemory, (LPVOID)this->m_hBaseAddress, this->m_nAddressSize);

#ifndef DEBUG
		LeaveCriticalSection(&this->m_lpCriticalSection);
#endif
		return bRet;
	}

	void ModifyBuffer(DWORD address, PBYTE value, uint32_t valueSize)
	{
#ifndef DEBUG
		EnterCriticalSection(&this->m_lpCriticalSection);
#endif

		DWORD bytesWrote = 0;
		WriteProcessMemory(GetCurrentProcess(), (LPVOID)address, value, valueSize, &bytesWrote);

		//This should fix our RVA to array issue.
		address -= (uint32_t)this->m_hBaseAddress;
		
		for(uint32_t i = address, j = 0; i < (address + valueSize); ++i, j++)
		{
			this->m_pMemory[i] = value[j];
		}

#ifndef DEBUG
		LeaveCriticalSection(&this->m_lpCriticalSection);
#endif		
	}


private:
	PBYTE m_pMemory;	
	HMODULE m_hBaseAddress;
	uint32_t m_nAddressSize;
#ifndef DEBUG
	CRITICAL_SECTION m_lpCriticalSection;
#endif
};