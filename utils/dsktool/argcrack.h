#ifndef __argcrack_h
#define __argcrack_h

#include <string>
#include <algorithm>

// (C) 2009 Charlie Robson

const char argsep = ':';

class pathutil
{
public:
	static bool changeExtension(std::string& filename, const char* newextension)
	{
		size_t expos = filename.find_last_of('.');
		bool dotfound = expos != std::string::npos;
		if (dotfound)
		{
			std::string base = filename.substr(0, expos + 1);

			if (newextension[0] == '.')
			{
				++newextension;
			}

			filename = base + newextension;
		}

		return dotfound;
	}

	static bool checkExtension(std::string& filename, const char* extension)
	{
		size_t expos = filename.find_last_of('.');
		bool dotfound = expos != std::string::npos;
		if (dotfound)
		{
			std::string extn = filename.substr(expos + 1);
			dotfound = _stricmp(extn.c_str(), extension) == 0;
		}

		return dotfound;
	}
private:
	pathutil(){};
	~pathutil(){};
};


class argcrack
{
public:
	argcrack(int argc, char **argv) :
	  m_argc(argc),
		  m_argv(argv)
	  {
	  }

	  bool split(const int index, std::string& left, std::string& right)
	  {
		  try
		  {
			  if (m_argc > index+1)
			  {
				  std::string ssarg(m_argv[index+1]);
				  size_t equoffs = ssarg.find_first_of(argsep);
				  if (equoffs != -1)
				  {
					  left = ssarg.substr(0, equoffs);
					  right = ssarg.substr(equoffs + 1, ssarg.length());
					  return true;
				  }
			  }
		  }
		  catch(...)
		  {
		  }

		  return false;
	  }

	  bool eval(const char* val, int& result)
	  {
		  int len = strlen(val);
		  if (len)
		  {
			  int base = 0;
			  if (*val == '%')
			  {
				  base = 2;
				  ++val;
			  }
			  else if (*val == '$')
			  {
				  base = 16;
				  ++val;
			  }
			  else if (len > 2 && val[0] == '0' && val[1] == 'x')
			  {
				  base = 16;
				  val+=2;
			  }

			  char* final;
			  result = strtol(val, &final, base);

			  if (*final == 0)
			  {
				  return true;
			  }
		  }

		  return false;
	  }

	  // pname should be of the form:
	  //
	  //  parm:
	  //
	  bool getint(const char* pname, int& target)
	  {
		  try
		  {
			  for(int i = 1; i < m_argc; ++i)
			  {
				  if (_strnicmp(pname,m_argv[i],strlen(pname)) == 0)
				  {
					  eval(&m_argv[i][strlen(pname)], target);

					  return true;
				  }
			  }
		  }
		  catch(...)
		  {
		  }

		  return false;
	  }


	  // pname should be of the form:
	  //
	  //  parm:
	  //
	  bool getstring(const char* pname, std::string& target)
	  {
		  try
		  {
			  for(int i = 1; i < m_argc; ++i)
			  {
				  if (_strnicmp(pname,m_argv[i],strlen(pname)) == 0)
				  {
					  target = &m_argv[i][strlen(pname)];
					  return true;
				  }
			  }
		  }
		  catch(...)
		  {
		  }

		  return false;
	  }


	  bool getat(int index, std::string& target)
	  {
		  bool enoughargs = m_argc > index;
		  if (enoughargs)
		  {
			  target = m_argv[index];
		  }

		  return enoughargs;
	  }


	  bool ispresent(const char* pname)
	  {
		  try
		  {
			  for(int i = 1; i < m_argc; ++i)
			  {
				  if (_strnicmp(pname,m_argv[i],strlen(pname)) == 0)
				  {
					  return true;
				  }
			  }
		  }
		  catch(...)
		  {
		  }

		  return false;
	  }

private:
	int m_argc;
	char** m_argv;
};

#endif
