#include <string>
#include <stdexcept>
#include <iostream>
#include <pire.h>

#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>

void Run(const Pire::Scanner& scanner, Pire::Scanner::State& state, const std::string& data)
{
	PIRE_IFDEBUG(std::clog << "---run---" << std::endl);
	Pire::Run(scanner, state, data.c_str(), data.c_str() + data.size());
}

bool ReadLine(FILE* f, std::string& str)
{
	int ch;
	str.clear();
	while ((ch = getc(f)) != EOF && ch != '\n')
		str.push_back((char) ch);
	return !str.empty() || ch != EOF;
}

int main(int argc, char** argv)
{
	try {
		if (argc < 2)
			throw std::runtime_error("Usage: pigrep <regexp> <options>");
		Pire::Lexer lexer(argv[1], argv[1] + strlen(argv[1]));
		bool surround = false;
		if (argc >= 3)
			for (const char* option = argv[2]; *option; ++option)
				if (*option == 'i')
					lexer.AddFeature(Pire::Features::CaseInsensitive());
				else if (*option == 'u')
					lexer.SetEncoding(Pire::Encodings::Utf8());
				else if (*option == 's')
					surround = true;
				else if (*option == 'a')
					lexer.AddFeature(Pire::Features::AndNotSupport());
				else
					throw std::runtime_error("Unknown option");
		Pire::Fsm fsm = lexer.Parse();
		if (surround)
			fsm.Surround();
		Pire::Scanner scanner(fsm);

		std::string str;
		while (ReadLine(stdin, str)) {
			PIRE_IFDEBUG(std::clog << "---run---" << std::endl);
			if (surround && Pire::Runner(scanner).Begin().Run(str).End()
				|| !surround && Pire::Runner(scanner).Run(str))
			{
				std::cout << str << std::endl;
			}
		}

		return 0;
	} catch (std::exception& e) {
		std::cerr << "pigrep: " << e.what() << std::endl;
		return 1;
	}
}