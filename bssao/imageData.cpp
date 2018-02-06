/*
 * imageData.cpp
 *
 *  Created on: 23.06.2014
 *      Author: Andreas Schuster
 */

#include "imageData.hh"

#include <ImfOutputFile.h>
#include <ImfMatrixAttribute.h>
#include <ImfTestFile.h>
#include <ImfChannelList.h>

namespace blech {

template<class T> T sqlen( Imath::Vec3<T> v)
{
	return v.x*v.x + v.y*v.y + v.z*v.z;
}


inline bool fuzzyZero( float x ) { return std::fabs(x) < __FLT_EPSILON__; }


blech::ImageData::ImageData():
m_quiet(false),
m_hasCameraMatrix(false),
m_hasScreenMatrix(false),
m_samples(32),
m_numDirs(16),
m_numRays(4),
m_numSteps(16)
{
    m_dirs = new Imath::V2f[m_numDirs];
    initDirections();
    m_steps = new double[m_numSteps];
    initSteps();
}

blech::ImageData::~ImageData()
{
    delete[] m_dirs;
    delete[] m_steps;
}

bool blech::ImageData::loadImage( const std::string &i_path )
{
    if ( !fileExists( i_path ) )
    {
        std::cerr << "ImageData::loadImage(): file does not exist: " << i_path.c_str() << std::endl;
        return false;
    }

    // check that the file is a valid EXR image
    if ( ! Imf::isOpenExrFile( i_path.c_str() ) )
    {
        std::cerr << "ExrData: Error: File is not a valid EXR image: " << i_path.c_str() << std::endl;
        return false;
    }

    m_imageHandleExr = new Imf::InputFile( i_path.c_str());

    // check that the file is complete
    if ( !m_imageHandleExr->isComplete() )
    {
        std::cerr << "ExrData: Error: File is incomplete: " << i_path.c_str() << std::endl;
        return false;
    }

    // Get Channel type
    const Imf::ChannelList &channels = m_imageHandleExr->header().channels();
    for ( Imf::ChannelList::ConstIterator i = channels.begin(); i != channels.end(); ++i )
    {
        if ( !m_quiet )
        {
            std::cout << "ExrData: \tChannelName: " << i.name();
            std::cout << "\ttype: " << static_cast<int>(i.channel().type) << std::endl;
        }

        if ( std::string(i.name()) == "R")
        {
            m_rType = i.channel().type;
        }
        else if ( std::string(i.name())  == "G")
        {
            m_gType = i.channel().type;
        }
        else if ( std::string(i.name())  == "B")
        {
            m_bType = i.channel().type;
        }
        else if ( std::string(i.name())  == "A")
        {
            m_aType = i.channel().type;
        }
        else if ( std::string(i.name())  == "Z")
        {
            m_zType = i.channel().type;
        }
    }

    // Get data window
    m_dw = m_imageHandleExr->header().dataWindow();
    m_width = m_dw.max.x - m_dw.min.x+1;
    m_height = m_dw.max.y - m_dw.min.y+1;

    if ( !m_quiet )
    {
        std::cout << "ExrData: \tData Window: " << m_width << " x " << m_height << std::endl;
    }

    // allocate
    m_rPixels.resizeErase( m_height, m_width );
    m_gPixels.resizeErase( m_height, m_width );
    m_bPixels.resizeErase( m_height, m_width );
    m_aPixels.resizeErase( m_height, m_width );
    m_zPixels.resizeErase( m_height, m_width );
    //m_rPixels = new Imf::Array2D<half>(); // m_height, m_width );


    // create framebuffer for access
    Imf::FrameBuffer frameBuffer;
    frameBuffer.insert( "R", Imf::Slice(  m_rType,
                                            (char*)(&m_rPixels[0][0]-m_dw.min.x-m_dw.min.y*m_width),
                                            sizeof( m_rPixels[0][0]) * 1,
                                            sizeof( m_rPixels[0][0]) * m_width,
                                            1,
                                            1,
                                            0.0 ) );
    frameBuffer.insert( "G", Imf::Slice(  m_gType,
                                            (char*)(&m_gPixels[0][0]-m_dw.min.x-m_dw.min.y*m_width),
                                            sizeof( m_gPixels[0][0]) * 1,
                                            sizeof( m_gPixels[0][0]) * m_width,
                                            1,
                                            1,
                                            0.0 ) );
    frameBuffer.insert( "B", Imf::Slice(  m_bType,
                                            (char*)(&m_bPixels[0][0]-m_dw.min.x-m_dw.min.y*m_width),
                                            sizeof( m_bPixels[0][0]) * 1,
                                            sizeof( m_bPixels[0][0]) * m_width,
                                            1,
                                            1,
                                            0.0 ) );
    frameBuffer.insert( "A", Imf::Slice(  m_aType,
                                            (char*)(&m_aPixels[0][0]-m_dw.min.x-m_dw.min.y*m_width),
                                            sizeof( m_aPixels[0][0]) * 1,
                                            sizeof( m_aPixels[0][0]) * m_width,
                                            1,
                                            1,
                                            0.0 ) );
    frameBuffer.insert( "Z", Imf::Slice(  m_zType,
                                            (char*)(&m_zPixels[0][0]-m_dw.min.x-m_dw.min.y*m_width),
                                            sizeof( m_zPixels[0][0]) * 1,
                                            sizeof( m_zPixels[0][0]) * m_width,
                                            1,
                                            1,
                                            __FLT_MAX__ ) );
    m_imageHandleExr->setFrameBuffer( frameBuffer );

    // read pixels
    m_imageHandleExr->readPixels( m_dw.min.y, m_dw.max.y );

    return readHeader( i_path );
}



bool blech::ImageData::saveImage( const std::string &i_path )
{
    Imf::Header header(m_width, m_height );

    header.channels().insert ("R", Imf::Channel (Imf::HALF));
    header.channels().insert ("G", Imf::Channel (Imf::HALF));
    header.channels().insert ("B", Imf::Channel (Imf::HALF));
    header.channels().insert ("A", Imf::Channel (Imf::HALF));

    Imf::OutputFile file(i_path.c_str(), header);
    Imf::FrameBuffer frameBuffer;

    frameBuffer.insert ("R", Imf::Slice( Imf::HALF,
                                        (char *) (&m_rPixels[0][0] -  m_dw.min.x - m_dw.min.y * m_width ),
                                        sizeof (m_rPixels[0][0]) * 1,        // xStride
                                        sizeof (m_rPixels[0][0]) * m_width,// yStride
                                        1,
                                        1,                                // x/y sampling
                                        0.0 ) );

    frameBuffer.insert ("G", Imf::Slice( Imf::HALF,                          // type
                                        (char *) (&m_gPixels[0][0] -  m_dw.min.x - m_dw.min.y * m_width ),
                                        sizeof (m_gPixels[0][0]) * 1,        // xStride
                                        sizeof (m_gPixels[0][0]) * m_width,// yStride
                                        1,
                                        1,                                // x/y sampling
                                        0.0 ));

    frameBuffer.insert ("B", Imf::Slice( Imf::HALF,                          // type
                                        (char *) (&m_bPixels[0][0] -  m_dw.min.x - m_dw.min.y * m_width ),
                                        sizeof (m_bPixels[0][0]) * 1,        // xStride
                                        sizeof (m_bPixels[0][0]) * m_width,// yStride
                                        1,
                                        1,                                // x/y sampling
                                        0.0 ));

    frameBuffer.insert ("A", Imf::Slice( Imf::HALF,                          // type
                                        (char *) (&m_aPixels[0][0] -  m_dw.min.x - m_dw.min.y * m_width ),
                                        sizeof (m_aPixels[0][0]) * 1,        // xStride
                                        sizeof (m_aPixels[0][0]) * m_width,// yStride
                                        1,
                                        1,                                // x/y sampling
                                        0.0 ));

    file.setFrameBuffer( frameBuffer );
    file.writePixels( m_height );

    return false;
}


void blech::ImageData::verifyZValues()
{
    int nonValidCounter = 0;
    for ( int width = 0; width < m_width; width++ )
    {
        for ( int height = 0; height < m_height; height++ )
        {
            half h = (half)m_zPixels[height][width];
            // check for infinite or nan z channel
            if ( h.isInfinity() || h.isNan() )
            {
                nonValidCounter++;
                m_zPixels[height][width] = (half)INT_MAX;
            }
        }
    }
}



bool blech::ImageData::processSSAO( double i_radius)
{
    if (!m_quiet)
    {
        std::cerr << "Start SSAO" << std::endl;
    }

    // allocate
    m_occlPixels.resizeErase( m_height, m_width  );


    std::srand( std::time( NULL ) );

    for ( int width = 0; width < m_width; width++ )
    {
        for ( int height = 0; height < m_height; height++ )
        {
            if ( (half)m_zPixels[height][width] == (half)INT_MAX )
            {
                m_occlPixels[height][width] = (half)1.0;
            }
            else
            {
                // calculate occlusion
                m_occlPixels[height][width] = occlusion( width, height, i_radius);
            }
        }
    }

    for ( int width = 0; width < m_width; width++ )
    {
        for ( int height = 0; height < m_height; height++ )
        {
            // no filter
            // half occ = m_occlPixels[height][width];
            // box filter
            half occ = 0.0;
            for ( int i=-1; i<2; i++)
            {
                for ( int j=-1; j<2; j++)
                {
                    int m = std::min( std::max(0,height+i), m_height-1);
                    int n = std::min( std::max(0,width+i), m_width-1);
                    occ += m_occlPixels[m][n];
                }
            }
            occ /= 9.0;
            m_rPixels[height][width] = m_rPixels[height][width] * occ; // + fog;
            m_gPixels[height][width] = m_gPixels[height][width] * occ; // + fog;
            m_bPixels[height][width] = m_bPixels[height][width] * occ; // + fog;
        }
    }

    return true;
}


bool blech::ImageData::processFog( double i_start, double i_end, double i_density )
{
    for ( int width = 0; width < m_width; width++ )
    {
        for ( int height = 0; height < m_height; height++ )
        {
            double x = std::min( (double)m_zPixels[height][width], i_end );
            // map to [0,density]
            double fogValue = ( x*i_density - i_start*i_density ) / ( i_end - i_start );

            m_rPixels[height][width] = (half)std::min( 1.0, (double)m_rPixels[height][width]+fogValue );
            m_gPixels[height][width] = (half)std::min( 1.0, (double)m_gPixels[height][width]+fogValue );
            m_bPixels[height][width] = (half)std::min( 1.0, (double)m_bPixels[height][width]+fogValue );
        }
    }

    return true;
}


bool blech::ImageData::readHeader( const std::string &i_path )
{

    const Imf::M44fAttribute* w2cAttr = m_imageHandleExr->header().findTypedAttribute<Imf::M44fAttribute>( "WorldToCamera" );

    if ( w2cAttr != NULL )
    {
        // Note that we're NOT reading it transposed!
        for( int i = 0; i < 4; ++i )
        {
            for( int j = 0; j < 4; ++j )
            {
                float v = w2cAttr->value()[i][j];
                m_cameraMatrix[i][j] = (fuzzyZero(v)) ? 0.0 : v;
            }
        }
        m_hasCameraMatrix = true;
        m_cameraToWorldMatrix = m_cameraMatrix;
        m_cameraToWorldMatrix.inverse();

        if ( !m_quiet )
        {
            std::cout << "ExrData:\tThis image contains a world => camera matrix." << std::endl;
            for( int i = 0; i < 4; ++i )
            {
                std::cout << m_cameraMatrix[i][0] << " " << m_cameraMatrix[i][1] << " " << m_cameraMatrix[i][2] << " " << m_cameraMatrix[i][3] << std::endl;
            }
        }

    }
    else
    {
        std::cout << "ExrData: Warning: \"WorldToCamera\" matrix not found in exr image." << std::endl;
    }

    const Imf::M44fAttribute *w2sAttr = m_imageHandleExr->header(). findTypedAttribute<Imf::M44fAttribute>( "WorldToScreen" );

    if ( w2sAttr != NULL )
    {
        // Note that we're NOT reading it TRANSPOSED!!!
        for( int i = 0; i < 4; ++i )
        {
            for( int j = 0; j < 4; ++j )
            {
                float v = w2sAttr->value()[i][j];
                m_screenMatrix[i][j] = (fuzzyZero(v)) ? 0.0 : v;
                m_WorldToScreen[i][j] = (fuzzyZero(v)) ? 0.0 : v;
            }
        }
        m_hasScreenMatrix = true;
        m_screenToWorldMatrix = m_screenMatrix;
        m_screenToWorldMatrix.inverse();

        if ( !m_quiet )
        {
            std::cout << "ExrData:\tThis image contains a world => screen matrix." << std::endl;
            //m_screenMatrix = m_screenMatrix.transpose();
            for( int i = 0; i < 4; ++i )
            {
                std::cout << m_screenMatrix[i][0] << " " << m_screenMatrix[i][1] << " " << m_screenMatrix[i][2] << " " << m_screenMatrix[i][3] << std::endl;
            }
        }

    }
    else
    {
        std::cout << "ExrData: Warning: matrix not found in exr image." << std::endl;
    }

    return true;
}



half blech::ImageData::occlusion( const int i_x, const int i_y, float i_radius )
{
    // uv in [0,1]
    double u = double(i_x) / (double) m_width;
    double v = double(m_height-i_y) / (double) m_height; // TODO ??

    // finest step in u and v
    Imath::V2d uvResolution = Imath::V2d( 1.0/m_width, 1.0/m_height );

    if ( i_x ==400 && i_y==100 && !m_quiet)
    {
        std::cout << "uvResolution: " << uvResolution.x << " " << uvResolution.y << std::endl;
    }

    // get eye space vector for P
    Imath::V3f P = getEyePos(u,v);

    // project the radius of influence to texture space
    Imath::V3f P_radius = Imath::V3f(P.x+i_radius, P.y+i_radius, P.z);
    // to screen
    P_radius *= m_cameraToWorldMatrix;
    P_radius *= m_screenMatrix;
    // [0,1]
    P_radius = Imath::V3f ( P_radius.x*0.5+0.5, P_radius.y*0.5+0.5, 0);
    double uv_radius = std::min(1.0, std::abs( u-P_radius.x) ) ;
    double numSteps = std::min( (double)m_numSteps, std::min( uv_radius*(double)m_width, uv_radius*(double)m_height ) );

    // Nearest neighbor pixels on the tangent plane ( siehe HorizonBasedAO )
    Imath::V3f Pr, Pl, Pt, Pb;
    Imath::V3f N; //TODO: get it from image and use normal
    Imath::V3f tangentPlane;
    // get a normal
    Pr = getEyePos(u+2*uvResolution.x, v);
    Pl = getEyePos(u-2*uvResolution.x, v);
    Pt = getEyePos(u, v+2*uvResolution.y);
    Pb = getEyePos(u, v-2*uvResolution.y);
    N = (Pr-Pl).cross( (Pt-Pb) ); //  normalize(cross(Pr - Pl, Pt - Pb));
    N.normalize();
    // tangentPlane = Imath::V4f(N.x, N.y, N.z, w_dot<float>(P,N) );

    // Screen-aligned basis for the tangent plane
    Imath::V3f dPdu = min_diff(P, Pr, Pl);
    Imath::V3f dPdv = min_diff(P, Pt, Pb); // * m_dswHeight  / m_dswWidth; // * (g_Resolution.y * g_InvResolution.x);
    dPdv *= (float)m_height;
    dPdv *= ( 1.0/ (float) m_width );

    float stepsize = uv_radius / numSteps;

    Imath::V2f randomDir = Imath::V2f( ((rand()%100 + 50)*0.01), ((rand()%100 + 50)*0.01) );

    if ( i_x ==400 && i_y==100 && !m_quiet)
    {
        std::cout << "i_radius: " << i_radius << std::endl;
        std::cout << "dPdu: " << dPdu.x << " " << dPdu.y << " " << dPdu.z << std::endl;
        std::cout << "dPdv: " << dPdv.x << " " << dPdv.y << " " << dPdv.z << std::endl;
        std::cout << "Normal: " << N.x << " " << N.y << " " << N.z << std::endl;
        std::cout << "uv_radius: " << uv_radius << std::endl;
        std::cout << "numSteps: " << numSteps << std::endl;
        std::cout << "stepsize: " << stepsize << std::endl;
        std::cout << "randomDir: " << randomDir.x << " " << randomDir.y << std::endl;
    }


    float squareradius = i_radius * i_radius;

    float tanAngleBias = 0.01;

    double occlusion = 0.0;

    for ( int i=0; i<m_numDirs; i++ )
    {
        // get 2d vector for current sample with random length within radius
    	Imath::V2f sample2DVector = Imath::V2f( m_dirs[i].x * randomDir.x, m_dirs[i].y * randomDir.y) ;
        sample2DVector *= uv_radius;

        // Jitter starting point within the first sample distance
        Imath::V2f uv = Imath::V2f(u,v) + sample2DVector * (float)m_steps[0]; // +  Imath::V2f(deltaUV.x * (rand()%100 + 1)*0.01f, deltaUV.y * (rand()%100 + 1)*0.01f);

        // Snap first sample uv and initialize horizon tangent
        Imath::V2f snapped_duv = snap_uv_offset(uv - Imath::V2f(u,v) );
        Imath::V3f T = tangent_vector(snapped_duv, dPdu, dPdv);

        float tanH = tangent(T) + tanAngleBias;

        float ao = 0.0;
        float h0 = 0.0;

        // AccumulatedHorizonOcclusion
        // for ( float j=0.0; j<numSteps; j++ )
        for ( int j=0; j<m_numSteps; j++ )
        {
            uv = Imath::V2f(u,v) + sample2DVector * (float)m_steps[j];

            Imath::V2f snapped_uv = snap_uv_coord( uv );

            // get eye space vector
            Imath::V3f S = getEyePos(snapped_uv.x, snapped_uv.y);

            // calc square distance between P and step point
            float dist = sqlen<float>(P-S);

            // ignore if to far
            if ( dist < squareradius)
            {
                float tanS = tangent(P, S);

                if (tanS > tanH)
                {
                    // Compute tangent vector associated with snapped_uv
                	Imath::V2f snapped_duv = snapped_uv - Imath::V2f(u,v);
                	Imath::V3f T = tangent_vector(snapped_duv, dPdu, dPdv);
                    float tanT = tangent(T) + tanAngleBias;

                    // Compute AO between tangent T and sample S
                    float sinS = tan_to_sin(tanS);
                    float sinT = tan_to_sin(tanT);
                    //float r = std::sqrt(dist) * ( 1.0 / i_radius); // g_inv_R;
                    float r = 1.0 - (dist / squareradius);
                    float h = sinS - sinT;
                    //ao += falloff(r) * (h - h0);
                    //ao += (1.0 - r*r) * (h - h0);
                    ao += r * (h - h0);
                    h0 = h;

                    // Update the current horizon angle
                    tanH = tanS;
                }
            }
        }

        occlusion += ao;
    }

    occlusion = 1.0 - occlusion / (double)m_numDirs;

    if ( i_x ==400 && i_y==100 && !m_quiet)
    {
        std::cout << "occlusion: " << occlusion << std::endl;
    }

    return (half) occlusion ;

}


Imath::V3f blech::ImageData::getEyePos(float i_u, float i_v)
{
    // get eye space vector for P, first shift u and v to [-1,1]
	Imath::V3f ES =  Imath::V3f((i_u-0.5)*2.0, (i_v-0.5)*2.0, 0.0);
    ES *= m_screenToWorldMatrix; // ndc to world
    ES *= m_cameraMatrix; // world to camera

    ES.normalize(); // norm it

    int x = std::max(0, std::min( (int)( i_u * (float)m_width ), m_width-1 ) );
    int y = std::max(0, std::min( m_height-(int)( i_v * (float)m_height), m_height-1) );

    float eyeZ = m_zPixels[m_height-y][x];

    if ( eyeZ == INFINITY )
    {
        eyeZ = INT_MAX;
    }

    eyeZ = (float)sampleZChannel(x ,y);
    // eyeZ = (float)sampleZChannel(i_u ,i_v);

    return Imath::V3f( ES.X*eyeZ, ES.Y*eyeZ, eyeZ );
}



Imath::V3f ImageData::min_diff(Imath::V3f P, Imath::V3f P1, Imath::V3f P2)
{
	Imath::V3f V1 = P1 - P;
	Imath::V3f V2 = P - P2;
    return (sqlen<float>(V1) < sqlen<float>(V2)) ? V1 : V2;
}


float ImageData::tangent(Imath::V3f T)
{
    return -T.Z * invlength(Imath::V2f(T.X,T.Y));
}

float ImageData::tangent(Imath::V3f P, Imath::V3f S)
{
    return (P.Z - S.Z) / (Imath::V2f(S.X, S.Y) - Imath::V2f(P.X, P.Y)).length2(); // TODO double check
}

Imath::V3f ImageData::tangent_vector(Imath::V2f deltaUV, Imath::V3f dPdu, Imath::V3f dPdv)
{
    return (deltaUV.X * dPdu + deltaUV.Y * dPdv) ;
}


Imath::V2f ImageData::snap_uv_offset(Imath::V2f uv)
{
    return  Imath::V2f ( std::floor(uv.X * (float)m_width) / (float)m_width, std::floor(uv.Y * (float)m_height) / (float)m_height);
}

Imath::V2f ImageData::snap_uv_coord(Imath::V2f uv)
{
    double intpart;
    // return uv - (frac(uv * g_Resolution) - 0.5f) * g_InvResolution;
    return  uv - Imath::V2f( (std::modf( uv.X * (float)m_width, &intpart ) - 0.5) / (float)m_width, (std::modf( uv.Y * (float)m_height, &intpart ) - 0.5) / (float)m_height);

}


float ImageData::rsqrt(float x)
{
  return std::pow(x, -0.5f);
}


float ImageData::invlength(Imath::V2f v)
{
    return rsqrt(w_dot<float>(v,v));
}


float ImageData::tan_to_sin(float x)
{
    return x * rsqrt(x*x + 1.0f);
}



void blech::ImageData::initDirections()
{
    // std::cout << "Directions:" << std::endl;
    double alpha = 2 * M_PI / (float)m_numDirs;
    for ( int i=0; i<m_numDirs; i++)
    {
        Imath::V2f v = Imath::V2f(std::cos( alpha * (double)i ), std::sin( alpha * (double)i ) );
        m_dirs[i] = v;
        // std::cout << i << " " << m_dirs[i].x << " " << m_dirs[i].y << std::endl;
    }
}

void blech::ImageData::initSteps()
{
    // std::cout << "Steps:" << std::endl;
    for ( int i=0; i<m_numSteps; i++ )
    {
        m_steps[i] = std::pow( ((double)i+1.0)/(double)m_numSteps, 2);
        // std::cout  << i << " " << m_steps[i] << std::endl;
    }
}

bool ImageData::fileExists( const std::string &i_path )
{
    struct stat stFileInfo;
    if ( stat(i_path.c_str(),&stFileInfo) != 0 )
    {
        return false;
    }
    return true;
}




}
