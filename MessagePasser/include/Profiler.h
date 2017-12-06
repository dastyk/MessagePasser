#ifndef _PROFILER_H_
#define _PROFILER_H_

#pragma once

#include <map>
#include <stdint.h>
#include <chrono>
#include <map>
#include <sstream>
#include <string>
#include <iostream>
#include <fstream>
#include <thread>
#include <mutex>
#include <iomanip>
#include <filesystem>

#ifdef _P_NS
static const char* scale = "ns";
#define _P_TIMESCALE std::chrono::nanoseconds
#else
#ifdef _P_MS
static const char* scale = "ms";
#define _P_TIMESCALE std::chrono::milliseconds
#else
static const char* scale = "ms";
#define _P_TIMESCALE std::chrono::milliseconds
#endif
#endif

#ifdef __PROFILE
class Profiler
{
	
	struct Data
	{
		Data() : timesCalled(0), timeSpent(0)
		{

		}
		Data(Data* parent, const char* functionName, uint64_t myHash) : parent(parent), functionName(functionName), myHash(myHash), timesCalled(0), timeSpent(0)
		{

		}
		~Data()
		{
			for (auto& c : children)
			{
				if (c.second)
					delete c.second;
			}
		}
		Data* parent;
		std::string functionName;
		uint64_t myHash;
		uint64_t timesCalled;
		std::chrono::nanoseconds timeSpent;
		std::chrono::high_resolution_clock::time_point timeStart;
		std::map<uint64_t, Data*> children;

		inline std::string getHexCode(unsigned char c) {


			std::stringstream ss;

			ss << std::uppercase << std::setw(2) << std::setfill('0') << std::hex;


			ss << +c;

			return ss.str();
		}
		const void dumpRoot(std::stringstream & out)
		{
			out << "\"" << this << "\"" << "[\n shape = none\n";
			out << "label = <<table border=\"0\" cellspacing = \"0\">\n";

			out << "<tr><td port=\"port1\" border=\"1\" bgcolor = \"#" << getHexCode(0) << getHexCode(150) << getHexCode(50) << "\"><font color=\"white\">" << functionName << "</font></td></tr>\n";
		
			for (auto& c : children)
			{
				timeSpent += c.second->timeSpent;
			}
			out << "<tr><td border=\"1\">" << "Time Spent: " << std::chrono::duration_cast<_P_TIMESCALE>(timeSpent).count() << " " << scale << "</td></tr>\n";

			out << "</table>>]\n";
			for (auto& c : children)
			{
				c.second->dump(out);
				out << "\"" << this << "\":port1 -> \"" << c.second << "\":port1\n";
			}
			out << "\n";
			return void();
		}
		const void dump(std::stringstream & out)
		{
			out << "\"" << this << "\"" << "[\n shape = none\n";
			out << "label = <<table border=\"0\" cellspacing = \"0\">\n";
			double div = 0.0;
			if (parent)
				div = ((double)timeSpent.count() / parent->timeSpent.count());
			out << "<tr><td port=\"port1\" border=\"1\" bgcolor = \"#" << getHexCode(unsigned char(150*div)) << getHexCode(50) << getHexCode(unsigned char(50 * (1.0-div))) << "\"><font color=\"white\">" << functionName << "</font></td></tr>\n";
			out << "<tr><td border=\"1\">" << "Times Called: " << timesCalled << "</td></tr>\n";
			out << "<tr><td border=\"1\">" << "Time Spent(IC): " << std::chrono::duration_cast<_P_TIMESCALE>(timeSpent).count() << " " << scale;
			if (parent)
				out << " " << div*100.0 << " % of parents.</td></tr>\n";
			else
				out << "</td></tr>\n";

			out << "<tr><td border=\"1\">" << "Time Spent(avg): " << std::chrono::duration_cast<_P_TIMESCALE>(timeSpent/timesCalled).count() << " " << scale << "</td></tr>\n";


			if (children.size())
			{
				auto temp = timeSpent;
				for (auto& c : children)
					temp -= c.second->timeSpent;
				out << "<tr><td border=\"1\">" << "Time Spent(EC): " << std::chrono::duration_cast<_P_TIMESCALE>(temp).count() << " " << scale << "</td></tr>\n";
			}

			out << "</table>>]\n";

			for (auto& c : children)
			{
				c.second->dump(out);
				out << "\"" << this << "\":port1 -> \"" << c.second << "\":port1\n";
			}
			out << "\n";
			return void();
		}

	};


private:


	Profiler()
	{

	}
	inline ~Profiler()
	{
		_dumpToFile();

		if (_profile)
		{
			delete _profile;
		}

	}



	Data* _profile = nullptr;
	Data* _current = nullptr;
public:
	inline static Profiler& GetInstance()
	{
		static thread_local Profiler inst;
		return inst;
	}
	template<uint64_t functionHash>
	const void StartProfileF(const char * funcName)
	{
		if (!_profile)
		{
			std::string name = funcName;
			size_t lastindex = name.find_last_of(":");
			if (lastindex == std::string::npos)
				_profile = _current = new Data(nullptr, "root", functionHash);
			else
			{
				size_t lastindex2 = name.substr(0, lastindex - 1).find_last_of(":");
				_profile = _current = new Data(nullptr, name.substr(0, lastindex- 1).substr(lastindex2 +1).c_str(), functionHash);
			}
				

			
		}
			
		{
			auto& child = _current->children[functionHash];
			if (!child)
				child = new Data(_current, funcName, functionHash);
			_current = child;	
		}	
		_current->timesCalled++;
		_current->timeStart = std::chrono::high_resolution_clock::now();
	}

	template<uint64_t functionHash>
	inline const void StopProfileF(const char * funcName)
	{
		std::chrono::high_resolution_clock::time_point time = std::chrono::high_resolution_clock::now();
		std::chrono::nanoseconds diff = time - _current->timeStart;
		if (_current->myHash != functionHash)
		{
			std::string asd = "Function mismatch. " + _current->functionName + " != " + funcName;
			throw std::exception(asd.c_str());
		}
		_current->timeSpent += diff;
		_current = _current->parent;
	}

private:


	const void _dumpToFile()
	{
		std::stringstream ss;

		ss << "digraph \"" << std::this_thread::get_id() << "\"{\n";
		ss << " rankdir = LR;\n";

		if (_profile)
		{
			_profile->dumpRoot(ss);
		}

		ss << "\n}\n";

		

		std::experimental::filesystem::create_directory("Profiler");
		std::experimental::filesystem::create_directory("Profiler\\" + _profile->functionName);
		std::ofstream bfile;
		bfile.open("Profiler\\ConvertDotsToPdf.bat", std::ios::trunc);
		if (bfile.is_open())
			bfile << R"(@if (@X)==(@Y) @end /* JScript comment
    @echo off

    set "extension=dot"

    setlocal enableDelayedExpansion
    for /R %%a in (*%extension%) do (
        for /f %%# in ('cscript //E:JScript //nologo "%~f0" %%a') do set "cdate=%%#"
       echo "%%~a"
	   echo "%%~dpa%%~na_!cdate!.pdf"
	   dot -Tpdf "%%~a" -o "%%~dpa%%~na_!cdate!.pdf"
	   del "%%~a"
    )

    rem cscript //E:JScript //nologo "%~f0" %*
    exit /b %errorlevel%
@if (@X)==(@Y) @end JScript comment */


FSOObj = new ActiveXObject("Scripting.FileSystemObject");
var ARGS = WScript.Arguments;
var file=ARGS.Item(0);

var d1=FSOObj.GetFile(file).DateCreated;

d2=new Date(d1);
var year=d2.getFullYear();
var mon=d2.getMonth();
var day=d2.getDate();
var h=d2.getHours();
var m=d2.getMinutes();
var s=d2.getSeconds();
var ms=d2.getMilliseconds();

if (mon<10){mon="0"+mon;}
if (day<10){day="0"+day;}
if (h<10){h="0"+h;}
if (m<10){m="0"+m;}
if (s<10){s="0"+s;}
if (ms<10){ms="00"+ms;}else if(ms<100){ms="0"+ms;}

WScript.Echo(""+year+mon+day+h+m+s+ms);)";


		bfile.close();


		std::ofstream rf;
		rf.open("Profiler\\BatchInstructions.txt", std::ios::trunc);
		if (rf.is_open())
			rf << R"(Download Graphviz, Ghostscript and GnuWin32

http://www.graphviz.org/pub/graphviz/stable/windows/graphviz-2.38.msi
https://github.com/ArtifexSoftware/ghostpdl-downloads/releases/download/gs921/gs921w64.exe
https://sourceforge.net/projects/getgnuwin32/files/getgnuwin32/0.6.30/GetGnuWin32-0.6.3.exe/download

Add C:\Program Files (x86)\Graphviz2.38\bin (or equivalent) to Path Environment variable
Add C:\Program Files\gs\gs9.21\lib (or equivalent) to Path Environment variable
C:\Program Files (x86)\GnuWin32\bin (or equivalent) to Path Environment variable
)";


		rf.close();
		std::ofstream out;
		std::stringstream fn;
		fn <<  "Profiler\\" << _profile->functionName << "\\profile_" << std::this_thread::get_id() << ".dot";
		out.open(fn.str(), std::ios::out | std::ios::trunc);
		if (!out.is_open())
			throw std::exception("Profile file could not be opened");
		out.write(ss.str().c_str(), ss.str().size());

		out.close();

		return void();
	}


};

static constexpr unsigned int crc_table[256] = {
	0x00000000, 0x77073096, 0xee0e612c, 0x990951ba, 0x076dc419, 0x706af48f,
	0xe963a535, 0x9e6495a3,    0x0edb8832, 0x79dcb8a4, 0xe0d5e91e, 0x97d2d988,
	0x09b64c2b, 0x7eb17cbd, 0xe7b82d07, 0x90bf1d91, 0x1db71064, 0x6ab020f2,
	0xf3b97148, 0x84be41de, 0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7,
	0x136c9856, 0x646ba8c0, 0xfd62f97a, 0x8a65c9ec, 0x14015c4f, 0x63066cd9,
	0xfa0f3d63, 0x8d080df5, 0x3b6e20c8, 0x4c69105e, 0xd56041e4, 0xa2677172,
	0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b, 0x35b5a8fa, 0x42b2986c,
	0xdbbbc9d6, 0xacbcf940, 0x32d86ce3, 0x45df5c75, 0xdcd60dcf, 0xabd13d59,
	0x26d930ac, 0x51de003a, 0xc8d75180, 0xbfd06116, 0x21b4f4b5, 0x56b3c423,
	0xcfba9599, 0xb8bda50f, 0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924,
	0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d, 0x76dc4190, 0x01db7106,
	0x98d220bc, 0xefd5102a, 0x71b18589, 0x06b6b51f, 0x9fbfe4a5, 0xe8b8d433,
	0x7807c9a2, 0x0f00f934, 0x9609a88e, 0xe10e9818, 0x7f6a0dbb, 0x086d3d2d,
	0x91646c97, 0xe6635c01, 0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e,
	0x6c0695ed, 0x1b01a57b, 0x8208f4c1, 0xf50fc457, 0x65b0d9c6, 0x12b7e950,
	0x8bbeb8ea, 0xfcb9887c, 0x62dd1ddf, 0x15da2d49, 0x8cd37cf3, 0xfbd44c65,
	0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2, 0x4adfa541, 0x3dd895d7,
	0xa4d1c46d, 0xd3d6f4fb, 0x4369e96a, 0x346ed9fc, 0xad678846, 0xda60b8d0,
	0x44042d73, 0x33031de5, 0xaa0a4c5f, 0xdd0d7cc9, 0x5005713c, 0x270241aa,
	0xbe0b1010, 0xc90c2086, 0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f,
	0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4, 0x59b33d17, 0x2eb40d81,
	0xb7bd5c3b, 0xc0ba6cad, 0xedb88320, 0x9abfb3b6, 0x03b6e20c, 0x74b1d29a,
	0xead54739, 0x9dd277af, 0x04db2615, 0x73dc1683, 0xe3630b12, 0x94643b84,
	0x0d6d6a3e, 0x7a6a5aa8, 0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1,
	0xf00f9344, 0x8708a3d2, 0x1e01f268, 0x6906c2fe, 0xf762575d, 0x806567cb,
	0x196c3671, 0x6e6b06e7, 0xfed41b76, 0x89d32be0, 0x10da7a5a, 0x67dd4acc,
	0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5, 0xd6d6a3e8, 0xa1d1937e,
	0x38d8c2c4, 0x4fdff252, 0xd1bb67f1, 0xa6bc5767, 0x3fb506dd, 0x48b2364b,
	0xd80d2bda, 0xaf0a1b4c, 0x36034af6, 0x41047a60, 0xdf60efc3, 0xa867df55,
	0x316e8eef, 0x4669be79, 0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236,
	0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f, 0xc5ba3bbe, 0xb2bd0b28,
	0x2bb45a92, 0x5cb36a04, 0xc2d7ffa7, 0xb5d0cf31, 0x2cd99e8b, 0x5bdeae1d,
	0x9b64c2b0, 0xec63f226, 0x756aa39c, 0x026d930a, 0x9c0906a9, 0xeb0e363f,
	0x72076785, 0x05005713, 0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38,
	0x92d28e9b, 0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21, 0x86d3d2d4, 0xf1d4e242,
	0x68ddb3f8, 0x1fda836e, 0x81be16cd, 0xf6b9265b, 0x6fb077e1, 0x18b74777,
	0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c, 0x8f659eff, 0xf862ae69,
	0x616bffd3, 0x166ccf45, 0xa00ae278, 0xd70dd2ee, 0x4e048354, 0x3903b3c2,
	0xa7672661, 0xd06016f7, 0x4969474d, 0x3e6e77db, 0xaed16a4a, 0xd9d65adc,
	0x40df0b66, 0x37d83bf0, 0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9,
	0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6, 0xbad03605, 0xcdd70693,
	0x54de5729, 0x23d967bf, 0xb3667a2e, 0xc4614ab8, 0x5d681b02, 0x2a6f2b94,
	0xb40bbe37, 0xc30c8ea1, 0x5a05df1b, 0x2d02ef8d
};


template<int size, int idx = 0, class dummy = void>
struct MM {
	static constexpr unsigned int crc32(const char * str, unsigned int prev_crc = 0xFFFFFFFF)
	{
		return MM<size, idx + 1>::crc32(str, (prev_crc >> 8) ^ crc_table[(prev_crc ^ str[idx]) & 0xFF]);
	}
};

// This is the stop-recursion function
template<int size, class dummy>
struct MM<size, size, dummy> {
	static constexpr unsigned int crc32(const char * str, unsigned int prev_crc = 0xFFFFFFFF)
	{
		return prev_crc ^ 0xFFFFFFFF;
	}
};


// This doesn't take into account the nul char
#define COMPILE_TIME_CRC32_STR(x) (MM<sizeof(x)-1>::crc32(x))

#define StartProfile Profiler::GetInstance().StartProfileF<COMPILE_TIME_CRC32_STR(__FUNCTION__)>(__FUNCTION__)

#define StopProfile Profiler::GetInstance().StopProfileF<COMPILE_TIME_CRC32_STR(__FUNCTION__)>(__FUNCTION__);

#define ProfileReturnVoid {Profiler::GetInstance().StopProfileF<COMPILE_TIME_CRC32_STR(__FUNCTION__)>(__FUNCTION__); return;}
#define ProfileReturnConst(x) {Profiler::GetInstance().StopProfileF<COMPILE_TIME_CRC32_STR(__FUNCTION__)>(__FUNCTION__); return x;}
#define ProfileReturnRef(x) {auto& e = x; Profiler::GetInstance().StopProfileF<COMPILE_TIME_CRC32_STR(__FUNCTION__)>(__FUNCTION__); return e;}
#define ProfileReturn(x) {auto e = x; Profiler::GetInstance().StopProfileF<COMPILE_TIME_CRC32_STR(__FUNCTION__)>(__FUNCTION__); return e;}

#else
#define StartProfile 

#define StopProfile 

#define ProfileReturnVoid {return;}
#define ProfileReturnConst(x) {return x;}
#define ProfileReturn(x) {return x;}
#endif


#endif