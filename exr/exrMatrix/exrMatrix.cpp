//============================================================================
// Name        : exrMatrix.cpp
// Author      : Andreas Schuster
// Version     :
// Copyright   : 
// Description : Command to write matrix in exr header
//============================================================================

#include <iostream>
using namespace std;

#include "imageData.hh"

#include "boost/program_options.hpp"

// create options
namespace opt = boost::program_options;

struct OptionsCmd
{
    // Quiet mode (no info messages).
    bool mQuiet;

    bool mRemove;
    std::string mName;
    std::string mMatrix;

    // Input file.
    std::string mInputFile;

    // Output file.
    std::string mOutputFile;

    // Option defaults
    OptionsCmd() :
        mQuiet(false),
        mRemove(false)
    {}

    // Print the command-line argument synopsis.
    static void PrintUsage( std::ostream& errs, const char* appName, const opt::options_description& options )
    {
        errs << "Usage: " << appName << " [options] filename" << std::endl << options << std::endl;
    }

    // Parse command-line arguments into Options struct.
    int Parse(int argc, const char** argv, std::ostream& errs)
    {
        const char* appName = argv[0];
        opt::options_description namedOptions("Options");

        namedOptions.add_options()
            ("help,h", "Show help")
            ("quiet,q", opt::value<bool>(&mQuiet)->zero_tokens(), "Quiet mode (no info messages)")
            ("input,i", opt::value<std::string>(&mInputFile), "Input exr file with z-channel")
            ("output,o", opt::value<std::string>(&mOutputFile), "Output file")
            ("remove,r", opt::value<bool>(&mRemove)->zero_tokens(), "Remove matrix attribute")
            ("name,n", opt::value<std::string>(&mName), "Named matrix attribute")
            ("matrix,m", opt::value<std::string>(&mMatrix), "Matrix as 16, space separated, float values within quotes")
            ;

        // arbitrary amount of inputs
        opt::positional_options_description positionalOptions;
        positionalOptions.add("input", -1);

        try
        {
            opt::variables_map options;
            opt::store(opt::command_line_parser(argc, const_cast<char**>(argv)).
                       options(namedOptions).positional(positionalOptions).run(), options);
            opt::notify(options);

            if (options.count("help"))
            {
                PrintUsage(errs, appName, namedOptions);
                return 1;
            }
        }
        catch (std::exception& e)
        {
            errs << "Invalid options: " << e.what() << std::endl;
            PrintUsage(errs, appName, namedOptions);
            return 1;
        }
        return 0;
    }
};

int main( int argc, const char **argv )
{
    // Parse command-line options.
    OptionsCmd options;
    int status = options.Parse(argc, argv, std::cerr);
    if (status != 0)
        return status;

    if (options.mInputFile.empty())
    {
        std::cout << "Please provide an input EXR file"<< std::endl;
        return 1;
    }

    if ( options.mOutputFile.empty() )
    {
    	options.mOutputFile = options.mInputFile;
    }

    if ( options.mName.empty() )
	{
		std::cout << "Please give a m44f attribute name."<< std::endl;
		return 1;
	}

	blech::ImageData myImage = blech::ImageData();
	myImage.setQuiet( options.mQuiet );

	myImage.loadImage( options.mInputFile );

    if ( options.mRemove && options.mMatrix.empty() )
    {
    	if ( !myImage.removeM44fAttribute( options.mName ) )
		{
    		std::cout << "Attribute not exists: " << options.mName << std::endl;
    		return 1;
		}
    }

    if ( !options.mMatrix.empty() )
    {
        std::vector<std::string> parts;
        boost::split( parts, options.mMatrix, boost::is_any_of(" "));
        if ( parts.size() != 16 )
        {
    		std::cout << "The number of matrix values must be exactly 16. ( found " << parts.size() << " )"<< std::endl;
    		return 1;
        }

        float matrixValues[4][4];
        unsigned short n = 0;
        for ( std::vector<std::string>::const_iterator i = parts.begin(); i != parts.end(); ++i, ++n )
        {
        	matrixValues[(int)(n/4.0)][n%4] = atof( i->c_str() ); // TODO: check and create exception ( currently any non-number get's converted to 0 )
        }

        if ( !myImage.setM44fAttribute( options.mName, matrixValues ) )
        {
        	return 1;
        }
    }

    if ( !options.mRemove && options.mMatrix.empty() )
    {
    	std::ostringstream matrixStream;
    	if ( myImage.getM44fAttribute( options.mName, matrixStream ) )
    	{
    		std::cout << matrixStream.str() << std::endl;
    	}
    }
    else
    {
		if ( !myImage.saveImage( options.mOutputFile ) )
		{
			return 1;
		}
    }

	return 0;
}
