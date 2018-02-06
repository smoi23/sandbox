/*
 * imageData.hh
 *
 *  Created on: 23.06.2014
 *  Author: Andreas Schuster
 *  Description: Class to work on exr image
 */

#ifndef IMAGEDATA_HH_
#define IMAGEDATA_HH_

//
#include <boost/foreach.hpp>
#include <boost/algorithm/string.hpp>
#include <vector>
#include <cstdlib>
#include <cmath>
#include <sys/types.h>
#include <sys/stat.h>
#include <iostream>
#include <sstream>
#include <fstream>
// open exr
#include <ImfArray.h>
#include <ImathBox.h>
#include <half.h>
#include <ImathVec.h>
#include <ImathMatrix.h>
#include <ImfInputFile.h>
#include <ImfPixelType.h>

namespace blech {

class ImageData
{

public:
    ImageData();
    virtual ~ImageData();

    bool loadImage( const std::string &i_path );
    bool saveImage( const std::string &i_path );
    bool removeM44fAttribute( const std::string &i_name );
    bool setM44fAttribute( const std::string &i_name, const float i_matrix[4][4]);
    bool getM44fAttribute( const std::string &i_name, std::ostringstream &o_data );

    void setQuiet( bool i_v ) { m_quiet = i_v; }
    bool getQuiet() { return m_quiet; }

private:
    bool fileExists( const std::string &i_path );
    bool readHeader();

private:
    bool m_quiet;
    Imf::InputFile* m_imageHandleExr;
    Imf::Header m_header;
    Imath::Box2i m_dw;
    int m_width;
    int m_height;
};


}


#endif /* IMAGEDATA_HH_ */
