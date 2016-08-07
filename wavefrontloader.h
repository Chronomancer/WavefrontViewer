/*
Simple Obj viewer for Windows using OpenGL and SDL
Can only read triangular faces, not polygonal, and does not draw textures

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

class OBJClass
{
  private:	
	float* mNormalBuffer;	// Stores the Normals
	float* mTextureBuffer;
	float* mVertexBuffer;	// Stores the points which make the object
	
	long* mIndexBufferN;
	long* mIndexBufferT;
	long* mIndexBufferV;
	 
	float mScale;
	float mVmax[3];
	float mVmin[3];	
	float mCenter[3];
	
	long mFaceCount;
	long mTexelCount;
	long mNormalCount;
	long mVertexCount;
	long mTotalConnectTriangles;	// Stores the total number of connected triangles
	
	void CalcScale();
	void RemakeNormals();
	void CreateNewNormals();
	void RemakeTextures();

	void CalcMaxMin();
	void CalcCenter();		
	
 public: 	
	OBJClass();
	~OBJClass();	
    int Load(wchar_t *fileName);	// Loads the model
	void Release();				// Release the model	 
	
	inline float* GetNormalBuffer(){return mNormalBuffer;};		
	inline float* GetTextureBuffer(){return mTextureBuffer;};
	inline float* GetVertexBuffer(){return mVertexBuffer;};
	inline long* GetIndexBufferV(){return mIndexBufferV;};	
	inline long GetTotalConnectTriangles(){return mTotalConnectTriangles;}; 	
	
	inline bool HasNormals(){return mNormalCount > 0;};		
}; 
