/*
 * imageData.hh
 *
 *  Created on: 23.06.2014
 *      Author: Andreas Schuster
 */

#ifndef IMAGEDATA_HH_
#define IMAGEDATA_HH_

#define COSINE_WEIGHTED 1
// system
#include <boost/foreach.hpp>
#include <boost/algorithm/string.hpp>
#include <vector>
#include <cstdlib>
#include <cmath>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <limits>
#include <ctime>
#include <iostream>
#include <sstream>
#include <fstream>


#include <ImathBox.h>
#include <half.h>
#include <ImathVec.h>
#include <ImathMatrix.h>


#include <ImfInputFile.h>
#include <ImfArray.h>
#include <ImfPixelType.h>

namespace blech {

class ImageData
{

public:
    ImageData();
    virtual ~ImageData();

    bool loadImage( const std::string &i_path );
    bool saveImage( const std::string &i_path );
    void verifyZValues();
    bool processSSAO( double i_radius );
    bool processFog( double i_start, double i_end, double i_density );

    double getAverageDistance();
    void getDepthDistances( double &io_min, double &io_averg, double &io_max );
    void setQuiet( bool i_v ) { m_quiet = i_v; }
    bool getQuiet() { return m_quiet; }

private:
    void initDirections();
    void initSteps();
    bool fileExists( const std::string &i_path );
    bool readHeader(const std::string &i_path);
    half occlusion( const int i_x, const int i_y, float i_radius);
    // half occlusionRay( const int i_x, const int i_y, float i_radius );
    Imath::V3f getEyePos(float i_u, float i_v);

    Imath::V3f min_diff(Imath::V3f P, Imath::V3f P1, Imath::V3f P2);
    float tangent(Imath::V3f T);
    float tangent(Imath::V3f P, Imath::V3f S);
    Imath::V3f tangent_vector(Imath::V2f deltaUV, Imath::V3f dPdu, Imath::V3f dPdv);
    Imath::V2f snap_uv_offset(Imath::V2f uv);
    Imath::V2f snap_uv_coord(Imath::V2f uv);
    float rsqrt(float x);
    float invlength(Imath::V2f v);
    float tan_to_sin(float x);
    half sampleZChannel( int i_x, int i_y);
    Imath::V3f fetchEyePos( const Imath::V3f &i_eyePos );

public:

private:
    bool m_quiet;

    // openExr
    Imf::InputFile* m_imageHandleExr;
    Imf::Array2D<half> m_rPixels;
    Imf::Array2D<half> m_gPixels;
    Imf::Array2D<half> m_bPixels;
    Imf::Array2D<half> m_aPixels;
    Imf::Array2D<half> m_zPixels;
    Imf::Array2D<float> m_occlPixels;
    Imath::Box2i m_dw;

    bool m_hasCameraMatrix;
    bool m_hasScreenMatrix;

    int m_width;
    int m_height;

    float m_samples;

    Imath::M33f m_cameraMatrix;
    Imath::M33f m_cameraToWorldMatrix;
    Imath::M33f m_screenMatrix;
    Imath::M33f m_screenToWorldMatrix;

    Imath::M44f m_WorldToScreen;

    const short m_numDirs;
    const short m_numRays;
    const short m_numSteps;

    Imath::V2f *m_dirs;
    double *m_steps;

    // channelTypes
    Imf::PixelType m_rType;
    Imf::PixelType m_gType;
    Imf::PixelType m_bType;
    Imf::PixelType m_aType;
    Imf::PixelType m_zType;

};


}


#endif /* IMAGEDATA_HH_ */
