#include <iostream>
#include <iterator>
#include <cctype>

using namespace std;

int main(int argc, char**)
{
	if(argc > 1) // yaa, got bitten passing args instead of piping
		std::cerr << "warning: too many arguments for titlize" << '\n';

	cin >> noskipws;

	istream_iterator<unsigned char> in(cin);
	ostream_iterator<unsigned char> out(cout);

	*out++ = toupper(*in++);

	bool just_skipped_space = false;
	for(; in != istream_iterator<unsigned char>(); ++in, ++out)
	{
		if(*in == '.')
			break;

		if(*in == '_' || isspace(*in))
		{
			just_skipped_space = true;
			continue;
		}

		if(just_skipped_space)
		{
			*out++ = ' ';
			*out = toupper(*in);
			just_skipped_space = false;
		}
		else
			*out = *in;
	}

	return 0;
}
