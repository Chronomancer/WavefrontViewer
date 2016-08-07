/*
Simple Obj viewer for Windows using OpenGL and SDL
Can only read triangular faces, not polygonal, and does not draw textures or materials

Modified code based on frank253's OpenGl Glut OBJ Loader sample
openglsamples.sourceforge.net/projects/index.pho/blog/index/
 
Wavefront loader is built on Ricardo Rendon Cepeda's OpenGL ES tutorial
raywenderlich.com/48293/

Wavefront loader also borrows elements from Lazaros Karydas's objview
github.com/lKarydas/objview

THE PROGRAM IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
IN THE SOFTWARE.
*/

#include <iostream>
#include <fstream>
#include <string>

#include <vector>
#include <cmath>

#include "wavefrontloader.h"

OBJClass::OBJClass()
{
	mNormalBuffer = NULL;							
	mTextureBuffer = NULL;
	mVertexBuffer = NULL;
	mIndexBufferV = NULL;
	mIndexBufferN = NULL;
	mIndexBufferT = NULL;
	mScale = 1.0f;
	mFaceCount = mTexelCount = mNormalCount = mVertexCount = mTotalConnectTriangles = 0;	
	mCenter[0] = mCenter[1] = mCenter[2] = 0.0f;
	mVmax[0] = mVmax[1] = mVmax[2] = 0.0f;
	mVmin[0] = mVmin[1] = mVmin[2] = 0.0f;
}

OBJClass::~OBJClass()
{
	Release();
}

//calculate mCenter of the 3D model using centroid of image moment equation
void OBJClass::CalcCenter()
{
	float m000 =0.0f;
	float m100 =0.0f;
	float m010 =0.0f;
	float m001 =0.0f;

	for (long i = 0; i < mVertexCount*4; i = i+4)
	{
		++m000;
		m100 = m100 + mVertexBuffer[i];
		m010 = m010 + mVertexBuffer[i+1];
		m001 = m001 + mVertexBuffer[i+2];
	}
	mCenter[0] = m100/m000;
	mCenter[1] = m010/m000;
	mCenter[2] = m001/m000;	
}

//*
int OBJClass::Load(wchar_t* fileName)
{ 
	mFaceCount = 0;
	//positioncount = 0;
	mTexelCount = 0;
	mNormalCount = 0;
	mVertexCount = 0;
	
	int itrP = 0, itrT = 0, itrN = 0, itrF = 0, count = 0, offset = 0;
	long fV = 0, fT = 0, fN = 0, fVmax = 0,fTmax = 0, fNmax = 0;
	std::string line, type;	
	char* nextToken;
	
	// Open OBJ file
    std::ifstream inOBJ;
    inOBJ.open(fileName);
    if(!inOBJ.good())
    {
        std::cout << "ERROR OPENING OBJ FILE" << std::endl;
		inOBJ.close();
        return -1;
    }
    
    // Read OBJ file, 1st pass
    while(!inOBJ.eof())
    {
        std::string line;
        std::getline(inOBJ, line);
        std::string type = line.substr(0,2);
        
        if(type.compare("v ") == 0)
            ++mVertexCount;
        else if(type.compare("vt") == 0)
            ++mTexelCount;
        else if(type.compare("vn") == 0)
            ++mNormalCount;
	    else if(type.compare("vt") == 0)
            ++mTexelCount;
        else if(type.compare("f ") == 0)
            ++mFaceCount;
    }    
   
	if ( mVertexCount == 0 || mFaceCount == 0)
		return -1;
   
    mVertexBuffer = new float[mVertexCount*4]();
	mIndexBufferV = new long[mFaceCount*3]();
	
	if (mNormalCount)
	{
		mNormalBuffer  = new float[mNormalCount*3]();
		mIndexBufferN = new long[mFaceCount*3]();	
	}
	if (mTexelCount)
	{
		mTextureBuffer  = new float[mTexelCount*2]();
		mIndexBufferT = new long[mFaceCount*3]();
    }
		
    inOBJ.clear();
	inOBJ.seekg (0, std::ios::beg);	
	
	//second pass
	while(!inOBJ.eof())
    {        
        std::getline(inOBJ, line);
        type = line.substr(0,2);
        
        // Positions
        if(type.compare("v ") == 0)
        {
            char* ln = new char[line.size()+1];
            memcpy(ln, line.c_str(), line.size()+1);
            
            // Extract tokens
            strtok_s(ln, " ", &nextToken);
			mVertexBuffer[4*itrP ] = (float) atof(strtok_s(NULL, " ", &nextToken));
			mVertexBuffer[4*itrP + 1] = (float) atof(strtok_s(NULL, " ", &nextToken));
			mVertexBuffer[4*itrP + 2] = (float) atof(strtok_s(NULL, " ", &nextToken));
            
			delete[] ln;
            ++itrP;
        }
        
        else if(type.compare("vt") == 0)
        {
            char* ln = new char[line.size()+1];
            memcpy(ln, line.c_str(), line.size()+1);
            
            strtok_s(ln, " ", &nextToken);
            mTextureBuffer[2*itrT ] = (float) atof(strtok_s(NULL, " ", &nextToken));
			mTextureBuffer[2*itrT + 1] = (float) atof(strtok_s(NULL, " ", &nextToken));
            
			delete[] ln;
            ++itrT;
        }
        
        // mNormalBuffer
        else if(type.compare("vn") == 0)
        {
            char* ln = new char[line.size()+1];
            memcpy(ln, line.c_str(), line.size()+1);
            
            strtok_s(ln, " ", &nextToken);
            mNormalBuffer[3*itrN ] = (float) atof(strtok_s(NULL, " ", &nextToken));
			mNormalBuffer[3*itrN + 1] = (float) atof(strtok_s(NULL, " ", &nextToken));
			mNormalBuffer[3*itrN + 2] = (float) atof(strtok_s(NULL, " ", &nextToken));
            
			delete[] ln;
            ++itrN;
        }
        
        // Faces
        else if(type.compare("f ") == 0)
        {
			const char* ln = line.c_str();
			fV = fT = fN = 0;		
			offset = count = 0;
			
			sscanf_s(ln, "%*s%n", &count);
			offset = count;

            if (strstr(ln, "//"))
            {
				sscanf_s(ln + offset, "%d//%d%n", &fV, &fN, &count);				
				mIndexBufferV[3*itrF ] = fV < 0 ? fV + mVertexCount -1: fV -1;
				mIndexBufferN[3*itrF ] = fN < 0 ? fN + mNormalCount -1: fN -1;	
				offset += count;

				sscanf_s(ln + offset, "%d//%d%n", &fV, &fN, &count);				
				mIndexBufferV[3*itrF + 1] = fV < 0 ? fV + mVertexCount -1: fV -1;
				mIndexBufferN[3*itrF + 1] = fN < 0 ? fN + mNormalCount -1: fN -1;	
				offset += count;

				sscanf_s(ln + offset, "%d//%d%n", &fV, &fN, &count);				
				mIndexBufferV[3*itrF + 2] = fV < 0 ? fV + mVertexCount -1: fV -1;
				mIndexBufferN[3*itrF + 2] = fN < 0 ? fN + mNormalCount -1: fN -1;	
				offset += count;
				//printf("%d %d %d\n", mIndexBufferV[3*itrF],mIndexBufferV[3*itrF+1],mIndexBufferV[3*itrF+2]);

            }
            else if (sscanf_s(ln + offset, "%d/%d/%d%n", &fV, &fT, &fN, &count) == 3)
            {
                mIndexBufferV[3*itrF ] = fV < 0 ? fV + mVertexCount -1: fV -1;
				mIndexBufferT[3*itrF ] = fT < 0 ? fT + mTexelCount -1: fT -1;
				mIndexBufferN[3*itrF ] = fN < 0 ? fN + mNormalCount -1: fN -1;	
				offset += count;

				sscanf_s(ln + offset, "%d/%d/%d%n", &fV, &fT, &fN, &count);				
				mIndexBufferV[3*itrF + 1] = fV < 0 ? fV + mVertexCount -1: fV -1;
				mIndexBufferT[3*itrF + 1] = fT < 0 ? fT + mTexelCount -1: fT -1;
				mIndexBufferN[3*itrF + 1] = fN < 0 ? fN + mNormalCount -1: fN -1;	
				offset += count;

				sscanf_s(ln + offset, "%d/%d/%d%n", &fV, &fT, &fN, &count);				
				mIndexBufferV[3*itrF + 2] = fV < 0 ? fV + mVertexCount -1: fV -1;
				mIndexBufferT[3*itrF + 2] = fT < 0 ? fT + mTexelCount -1: fT -1;
				mIndexBufferN[3*itrF + 2] = fN < 0 ? fN + mNormalCount -1: fN -1;	
				offset += count;
            }
			else if (sscanf_s(ln + offset, "%d/%d%n", &fV, &fT, &count ) == 2)
			{
				mIndexBufferV[3*itrF ] = fV < 0 ? fV + mVertexCount -1: fV -1;
				mIndexBufferT[3*itrF ] = fT < 0 ? fT + mTexelCount -1: fT -1;
				offset += count;

				sscanf_s(ln + offset, "%d/%d%n", &fV, &fT, &count );				
				mIndexBufferV[3*itrF + 1] = fV < 0 ? fV + mVertexCount -1: fV -1;
				mIndexBufferT[3*itrF + 1] = fT < 0 ? fT + mTexelCount -1: fT -1;
				offset += count;

				sscanf_s(ln + offset, "%d/%d%n", &fV, &fT, &count );				
				mIndexBufferV[3*itrF + 2] = fV < 0 ? fV + mVertexCount -1: fV -1;
				mIndexBufferT[3*itrF + 2] = fT < 0 ? fT + mTexelCount -1: fT -1;
				offset += count;
			}
			else
			{
				sscanf_s(ln + offset, "%d%n", &fV, &count );
				mIndexBufferV[3*itrF ] = fV < 0 ? fV + mVertexCount -1: fV -1;
				offset += count;

				sscanf_s(ln + offset, "%d%n", &fV, &count );				
				mIndexBufferV[3*itrF + 1] = fV < 0 ? fV + mVertexCount -1: fV-1;
				offset += count;

				sscanf_s(ln + offset, "%d%n", &fV, &count );			
				mIndexBufferV[3*itrF + 2] = fV < 0 ? fV + mVertexCount -1: fV-1;
				offset += count;				
			}
            ++itrF;
        }
    }	
	
	fVmax = fTmax = fNmax = 0;

	//find the maximum index in the faces
	for(long i=0; i < mFaceCount*3; ++i)
	{
		if (mIndexBufferV[i] > fVmax )
			fVmax = mIndexBufferV[i];
		
		if (mTexelCount > 0)
		{
			if (mIndexBufferT[i] > fTmax )
				fTmax = mIndexBufferT[i];
		}
		if (mNormalCount > 0)
		{
			if (mIndexBufferN[i] > fNmax )
				fNmax = mIndexBufferN[i];
		}
	}

	//if maximum index in the faces are larger than my vertex/normal/texel counts, 
	//then this model is missing data
	if ( fVmax > mVertexCount || (mNormalCount > 0 && fNmax > mNormalCount) || (mTexelCount > 0 && fTmax > mTexelCount) )
	{
		return -1;
	}

	mTotalConnectTriangles = mFaceCount*3;
	
	CalcMaxMin();
	CalcScale();
	//CalcCenter();
	
	for (long i = 0; i < mVertexCount*4; i = i+4)
	{
		mVertexBuffer[i + 3] = mScale;
	}
	
	RemakeNormals();
	//RemakeTextures();
	    
	// Close OBJ file
    inOBJ.close();
    
    return 0;
}

void OBJClass::CalcMaxMin()
{
	mVmax[0] = mVmax[1] = mVmax[2] = 0.0f;
	mVmin[0] = mVmin[1] = mVmin[2] = 0.0f;

	for (long i = 0; i < mVertexCount*4; i = i+4)
	{
		if ( mVertexBuffer[i] > mVmax[0])
			mVmax[0] = mVertexBuffer[i];
		if (mVertexBuffer[i+1] > mVmax[1])
			mVmax[1] = mVertexBuffer[i+1];
		if (mVertexBuffer[i+2] > mVmax[2])
			mVmax[2] = mVertexBuffer[i+2];
		
		if ( mVertexBuffer[i] < mVmin[0])
			mVmin[0] = mVertexBuffer[i];
		if (mVertexBuffer[i+1] < mVmin[1])
			mVmin[1] = mVertexBuffer[i+1];
		if (mVertexBuffer[i+2] < mVmin[2])
			mVmin[2] = mVertexBuffer[i+2];
	}	
}

void OBJClass::CalcScale()
{
	float scaletemp;
	
	scaletemp = mVmax[0] - mVmin[0];
	mScale = scaletemp * scaletemp;
	
	scaletemp = mVmax[1] - mVmin[1];
	mScale += scaletemp * scaletemp;
	
	scaletemp = mVmax[2] - mVmin[2];
	mScale += scaletemp * scaletemp;
	
	mScale = sqrt(mScale)/7.2f;
}

void OBJClass::RemakeNormals()
{
	bool bRemakeNormal = false;
	
	if ( mNormalCount > 0)
	{
		if ( mVertexCount != mNormalCount )
		{
			bRemakeNormal = true;
		}
		else
		{
			for(long i=0; i < mFaceCount; ++i)
			{
				if (mIndexBufferV[i] != mIndexBufferN[i])
				{
					bRemakeNormal= true;
					break;
				}
			}
		}	
	}
	else
	{
		CreateNewNormals();
	}	
	
	if (bRemakeNormal)
	{
		float* newnormalbuffer = new float[mVertexCount*3]();
		
		for(long i=0; i < mFaceCount; ++i)
		{
			newnormalbuffer[mIndexBufferV[i]] = mNormalBuffer[mIndexBufferN[i]];
			newnormalbuffer[mIndexBufferV[i]+1] = mNormalBuffer[mIndexBufferN[i]+1];
			newnormalbuffer[mIndexBufferV[i]+2] = mNormalBuffer[mIndexBufferN[i]+2];
		}
		
		delete[] mNormalBuffer;
		
		mNormalBuffer = newnormalbuffer;
	}
}

void OBJClass::CreateNewNormals()
{
	float edge1[3], edge2[3], normal[3], length;
	unsigned int i0, i1, i2;
	
	if ( mNormalBuffer == NULL)
		mNormalBuffer  = new float[mVertexCount*3]();
	
	for (long i = 0; i < mFaceCount*3; i = i+3)
    {
        i0 = mIndexBufferV[i];
		i1 = mIndexBufferV[i+1];
		i2 = mIndexBufferV[i+2];

        // Calculate triangle face normal.

        edge1[0] = mVertexBuffer[4*i1] - mVertexBuffer[4*i0]; 
        edge1[1] = mVertexBuffer[4*i1 +1] - mVertexBuffer[4*i0 +1];
        edge1[2] = mVertexBuffer[4*i1 +2] - mVertexBuffer[4*i0 +2];

        edge2[0] = mVertexBuffer[4*i2] - mVertexBuffer[4*i0];
        edge2[1] = mVertexBuffer[4*i2 +1] - mVertexBuffer[4*i0 +1];
        edge2[2] = mVertexBuffer[4*i2 +2] - mVertexBuffer[4*i0 +2];

        normal[0] = (edge1[1] * edge2[2]) - (edge1[2] * edge2[1]);
        normal[1] = (edge1[2] * edge2[0]) - (edge1[0] * edge2[2]);
        normal[2] = (edge1[0] * edge2[1]) - (edge1[1] * edge2[0]);

        // Accumulate the normals.

        mNormalBuffer[3*i0] += normal[0];
        mNormalBuffer[3*i0+1] += normal[1];
        mNormalBuffer[3*i0+2] += normal[2];

        mNormalBuffer[3*i1] += normal[0];
        mNormalBuffer[3*i1+1] += normal[1];
        mNormalBuffer[3*i1+2] += normal[2];

        mNormalBuffer[3*i2] += normal[0];
        mNormalBuffer[3*i2+1] += normal[1];
        mNormalBuffer[3*i2+2] += normal[2];
    }

    // Normalize the vertex normals.
    for (long i = 0; i < mVertexCount*3; i = i+3)
    {
        length = sqrtf(mNormalBuffer[i] * mNormalBuffer[i] +
            mNormalBuffer[i+1] * mNormalBuffer[i+1] +
            mNormalBuffer[i+2] * mNormalBuffer[i+2]);

        mNormalBuffer[i] /= length;
        mNormalBuffer[i+1] /= length;
        mNormalBuffer[i+2] /= length;
    }
	
	mNormalCount = mVertexCount;
}

void OBJClass::RemakeTextures()
{
	bool bRemakeTexture = false;	
	
	if ( mTexelCount > 0)
	{
		if ( mVertexCount != mTexelCount )
		{
			bRemakeTexture = true;
		}
		else
		{
			for(long i=0; i < mFaceCount; ++i)
			{
				if (mIndexBufferV[i] != mIndexBufferT[i])
				{
					bRemakeTexture= true;
					break;
				}
			}
		}
	}	
	
	if (bRemakeTexture)
	{
		float* newtexturebuffer = new float[mVertexCount*2]();
		
		for(long i=0; i < mFaceCount; ++i)
		{
			newtexturebuffer[mIndexBufferV[i]] = mTextureBuffer[mIndexBufferT[i]];
			newtexturebuffer[mIndexBufferV[i]+1] = mTextureBuffer[mIndexBufferT[i]+1];
		}
		
		delete[] mTextureBuffer;
		
		mTextureBuffer = newtexturebuffer;
		newtexturebuffer = NULL;
	}
}
 
void OBJClass::Release()
{
    if (this->mNormalBuffer!=NULL)
	{
        delete[] mNormalBuffer;
		mNormalBuffer = NULL;
	}
    if (this->mVertexBuffer!=NULL)
	{
        delete[] mVertexBuffer;
		mVertexBuffer = NULL;
	}
	if (this->mTextureBuffer!=NULL)
	{
        delete[] mTextureBuffer;
		mTextureBuffer = NULL;
	}

	if (this->mIndexBufferV!=NULL)
	{
        delete[] mIndexBufferV;
		mIndexBufferV = NULL;
	}
	if (this->mIndexBufferN!=NULL)
	{
        delete[] mIndexBufferN;	
		mIndexBufferN = NULL; 
	}
	if (this->mIndexBufferT!=NULL)
	{
        delete[] mIndexBufferT;	
		mIndexBufferT = NULL;
	}
}
