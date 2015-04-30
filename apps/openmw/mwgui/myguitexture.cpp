#include "myguitexture.hpp"

#include <stdexcept>
#include <iostream>

#include <osg/Texture2D>

#include <components/resource/texturemanager.hpp>

namespace MWGui
{

    OSGTexture::OSGTexture(const std::string &name, Resource::TextureManager* textureManager)
      : mName(name)
      , mTextureManager(textureManager)
      , mFormat(MyGUI::PixelFormat::Unknow)
      , mUsage(MyGUI::TextureUsage::Default)
      , mNumElemBytes(0)
    {
    }

    OSGTexture::OSGTexture(osg::Texture2D *texture)
        : mTextureManager(NULL)
        , mTexture(texture)
        , mFormat(MyGUI::PixelFormat::Unknow)
        , mUsage(MyGUI::TextureUsage::Default)
        , mNumElemBytes(0)
    {
    }

    OSGTexture::~OSGTexture()
    {
    }

    void OSGTexture::createManual(int width, int height, MyGUI::TextureUsage usage, MyGUI::PixelFormat format)
    {
        GLenum glfmt = GL_NONE;
        size_t numelems = 0;
        switch(format.getValue())
        {
            case MyGUI::PixelFormat::L8:
                glfmt = GL_LUMINANCE;
                numelems = 1;
                break;
            case MyGUI::PixelFormat::L8A8:
                glfmt = GL_LUMINANCE_ALPHA;
                numelems = 2;
                break;
            case MyGUI::PixelFormat::R8G8B8:
                glfmt = GL_RGB;
                numelems = 3;
                break;
            case MyGUI::PixelFormat::R8G8B8A8:
                glfmt = GL_RGBA;
                numelems = 4;
                break;
        }
        if(glfmt == GL_NONE)
            throw std::runtime_error("Texture format not supported");

        mTexture = new osg::Texture2D();
        mTexture->setTextureSize(width, height);
        mTexture->setSourceFormat(glfmt);
        mTexture->setSourceType(GL_UNSIGNED_BYTE);

        mTexture->setFilter(osg::Texture::MIN_FILTER, osg::Texture::LINEAR);
        mTexture->setFilter(osg::Texture::MAG_FILTER, osg::Texture::LINEAR);
        mTexture->setWrap(osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_EDGE);
        mTexture->setWrap(osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_EDGE);

        mFormat = format;
        mUsage = usage;
        mNumElemBytes = numelems;
    }

    void OSGTexture::destroy()
    {
        mTexture = nullptr;
        mFormat = MyGUI::PixelFormat::Unknow;
        mUsage = MyGUI::TextureUsage::Default;
        mNumElemBytes = 0;
    }

    void OSGTexture::loadFromFile(const std::string &fname)
    {
        if (!mTextureManager)
            throw std::runtime_error("No texturemanager set");

        mTexture = mTextureManager->getTexture2D(fname, osg::Texture2D::CLAMP, osg::Texture2D::CLAMP);

        // FIXME
        mFormat = MyGUI::PixelFormat::R8G8B8;
        mUsage = MyGUI::TextureUsage::Static | MyGUI::TextureUsage::Write;
        mNumElemBytes = 3; // FIXME
    }

    void OSGTexture::saveToFile(const std::string &fname)
    {
        std::cerr << "Would save image to file " << fname << std::endl;
    }

    int OSGTexture::getWidth()
    {
        if(!mTexture.valid())
            return 0;
        osg::Image *image = mTexture->getImage();
        if(image) return image->s();
        return mTexture->getTextureWidth();
    }

    int OSGTexture::getHeight()
    {
        if(!mTexture.valid())
            return 0;
        osg::Image *image = mTexture->getImage();
        if(image) return image->t();
        return mTexture->getTextureHeight();
    }

    void *OSGTexture::lock(MyGUI::TextureUsage /*access*/)
    {
        if (!mTexture.valid())
            throw std::runtime_error("Texture is not created");
        if (mLockedImage.valid())
            throw std::runtime_error("Texture already locked");

        mLockedImage = mTexture->getImage();
        if(!mLockedImage.valid())
        {
            mLockedImage = new osg::Image();
            mLockedImage->allocateImage(
                mTexture->getTextureWidth(), mTexture->getTextureHeight(), mTexture->getTextureDepth(),
                mTexture->getSourceFormat(), mTexture->getSourceType()
            );
        }
        return mLockedImage->data();
    }

    void OSGTexture::unlock()
    {
        if (!mLockedImage.valid())
            throw std::runtime_error("Texture not locked");

        // Tell the texture it can get rid of the image for static textures (since
        // they aren't expected to update much at all).
        mTexture->setImage(mLockedImage.get());
        mTexture->setUnRefImageDataAfterApply(mUsage.isValue(MyGUI::TextureUsage::Static) ? true : false);
        mTexture->dirtyTextureObject();

        mLockedImage = nullptr;
    }

    bool OSGTexture::isLocked()
    {
        return mLockedImage.valid();
    }

    // Render-to-texture not currently implemented.
    MyGUI::IRenderTarget* OSGTexture::getRenderTarget()
    {
        return nullptr;
    }

}