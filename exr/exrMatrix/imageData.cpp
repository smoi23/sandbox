/*
 * imageData.cpp
 *
 *  Created on: 23.06.2014
 *  Author: Andreas Schuster
 *  Description: Class to work on exr image
 */

#include "imageData.hh"

// open exr
#include <ImfForward.h>
#include <ImfOutputFile.h>
#include <ImfMatrixAttribute.h>
#include <ImfTestFile.h>
#include <ImfChannelList.h>
#include <ImfTileDescription.h>
#include <ImfTileDescriptionAttribute.h>


namespace blech {

// typedef boost::shared_ptr<Imf::Array2D<half> > pixelPtr;

blech::ImageData::ImageData():
m_quiet(false)
{}

blech::ImageData::~ImageData()
{}

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

    // read the header
    return readHeader();
}

bool blech::ImageData::saveImage( const std::string &i_path )
{
    if ( !m_quiet )
    {
        std::cout << "Writing image: " << i_path << std::endl;
    }

    // create framebuffer for access
    Imf::FrameBuffer frameBuffer;

    // list of pointers
	std::vector<Imf::Array2D<half>* > pixelPointers;

    // for every channel create a slice and put data in framebuffer
    const Imf::ChannelList &channels = m_imageHandleExr->header().channels();
    for (Imf::ChannelList::ConstIterator i = channels.begin(); i != channels.end(); ++i)
    {
    	if (!m_quiet)
    		std::cout << "Channel name: " << i.name() << " and type: " << static_cast<int>(i.channel().type) << std::endl;

    	Imf::Array2D<half> *pixels = new Imf::Array2D<half>;

    	// allocate
    	pixels->resizeErase( m_height, m_width );

	    frameBuffer.insert( i.name(), Imf::Slice(  i.channel().type,
	                        (char*)(pixels[0][0]-m_dw.min.x-m_dw.min.y*m_width),
	                        sizeof( (*pixels))[0][0] * 1,
	                        sizeof( (*pixels))[0][0] * m_width,
	                        1,
	                        1,
	                        0.0 ) );
		pixelPointers.push_back( pixels );
    }

    m_imageHandleExr->setFrameBuffer( frameBuffer );

    // read pixels
    m_imageHandleExr->readPixels( m_dw.min.y, m_dw.max.y );

    if (m_imageHandleExr->header().hasTileDescription() )
    {
    	if ( !m_quiet )
    		std::cout << "Write tiled image." << std::endl;

    	// const Imf::TileDescriptionAttribute *tileDescAttr = m_imageHandleExr->header().findTypedAttribute<Imf::TileDescriptionAttribute>( "tiles" );
    	// Imf::TileDescription tileDesc = tileDescAttr->value();
    	Imf::TiledOutputFile file( i_path.c_str(), m_header );

    	file.setFrameBuffer( frameBuffer );
        file.writeTiles(0, file.numXTiles()-1, 0, file.numYTiles()-1 );
    }
    else
    {
    	if ( !m_quiet )
    		std::cout << "Write scanlined image." << std::endl;

    	Imf::OutputFile file (i_path.c_str(), m_header);

        file.setFrameBuffer( frameBuffer );
        file.writePixels( m_height );
    }
	return true;
}

bool blech::ImageData::removeM44fAttribute( const std::string &i_name )
{
    if ( !m_quiet )
        std::cout << "Remove Attribute: " << i_name << std::endl;

    // const Imf::M44fAttribute* w2cAttr =
    if (m_imageHandleExr->header().findTypedAttribute<Imf::M44fAttribute>( i_name ) )
    {
    	m_header.erase( i_name );
    }
    else
    {
        return false;
    }
    return true;
}

bool blech::ImageData::setM44fAttribute( const std::string &i_name, const float i_matrix[4][4] )
{
	const Imath::M44f m44f( i_matrix );

	try
	{
		m_header.insert( i_name, Imf::M44fAttribute( m44f ) );
	}
	catch ( Iex::TypeExc &e )
	{
		std::cout << "Fail to insert attribute: " << e.what() << std::endl;
		return false;
	}

    if ( !m_quiet )
    {
    	std::cout << "Set Matrix:" << std::endl;
    	for ( unsigned short m=0; m<4; ++m )
    		std::cout << i_matrix[m][0] << " " << i_matrix[m][1] << " " << i_matrix[m][2] << " " << i_matrix[m][3] << " " << std::endl;
    }
	return true;
}

bool blech::ImageData::getM44fAttribute( const std::string &i_name, std::ostringstream &o_data )
{
    const Imf::M44fAttribute* m44fAttr = m_imageHandleExr->header().findTypedAttribute<Imf::M44fAttribute>( i_name );

    if ( m44fAttr )
    {
        const Imath::M44f m44 = m44fAttr->value();
    	for ( unsigned short m=0; m<4; ++m )
    		o_data << m44[m][0] << " " << m44[m][1] << " " << m44[m][2] << " " << m44[m][3] << " ";
    }
    else
    {
    	return false;
    }
    return true;
}

bool ImageData::readHeader()
{
    // Get data window
    m_dw = m_imageHandleExr->header().dataWindow();
    m_width = m_dw.max.x - m_dw.min.x+1;
    m_height = m_dw.max.y - m_dw.min.y+1;

    if ( !m_quiet )
        std::cout << "ExrData: \tData Window: " << m_width << " x " << m_height << std::endl;

    m_header = m_imageHandleExr->header();

	return true;
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


} // class
