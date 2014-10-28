

namespace octet {
  class PerlinNoise
  {
    private:

	float persistence, frequency, amplitude;
    int octaves, randomseed;

    public:

    PerlinNoise() {
      persistence = 0;
      frequency = 0;
      amplitude  = 0;
      octaves = 0;
      randomseed = 0;
    }

    PerlinNoise(float _persistence, float _frequency, float _amplitude, int _octaves, int _randomseed) {
      persistence = _persistence;
      frequency = _frequency;
      amplitude  = _amplitude;
      octaves = _octaves;
      randomseed = 2 + _randomseed * _randomseed;
    }

    float Persistence() const { return persistence; }
    float Frequency()   const { return frequency;   }
    float Amplitude()   const { return amplitude;   }
    int   Octaves()     const { return octaves;     }
    int   RandomSeed()  const { return randomseed;  }
  
    void SetPersistence(float _persistence) { persistence = _persistence; }
    void SetFrequency(  float _frequency)   { frequency = _frequency;     }
    void SetAmplitude(  float _amplitude)   { amplitude = _amplitude;     }
    void SetOctaves(    int    _octaves)     { octaves = _octaves;         }
    void SetRandomSeed( int    _randomseed)  { randomseed = 2 + _randomseed * _randomseed;   }

    void Set(float _persistence, float _frequency, float _amplitude, int _octaves, int _randomseed) {
      persistence = _persistence;
      frequency = _frequency;
      amplitude  = _amplitude;
      octaves = _octaves;
      randomseed = 2 + _randomseed * _randomseed;
    }
    
    // NOTE x and y should be positive values
    float GetHeight(float x, float y) const {
      return Total(x, y);
    }

    // NOTE i and j should be positive values
    float Total(float i, float j) const {
      //properties of one octave (changing each loop)
      float t = 0.0f;
      float _amplitude = amplitude;
      float freq = frequency;
      
      for(int k = 0; k < octaves; k++) 
      {
        t += GetValue(j * freq + randomseed, i * freq + randomseed) * _amplitude;
        _amplitude *= persistence;
        freq *= 2;
      }

      return t;
    }

    //values between -1 and 1
    float GetValue(float x, float y) const {
      int Xint = (int)x;
      int Yint = (int)y;
      float Xfrac = x - Xint;
      float Yfrac = y - Yint;

      //noise values
      float n01 = Noise(Xint-1, Yint-1);
      float n02 = Noise(Xint+1, Yint-1);
      float n03 = Noise(Xint-1, Yint+1);
      float n04 = Noise(Xint+1, Yint+1);
      float n05 = Noise(Xint-1, Yint);
      float n06 = Noise(Xint+1, Yint);
      float n07 = Noise(Xint, Yint-1);
      float n08 = Noise(Xint, Yint+1);
      float n09 = Noise(Xint, Yint);

      float n12 = Noise(Xint+2, Yint-1);
      float n14 = Noise(Xint+2, Yint+1);
      float n16 = Noise(Xint+2, Yint);

      float n23 = Noise(Xint-1, Yint+2);
      float n24 = Noise(Xint+1, Yint+2);
      float n28 = Noise(Xint, Yint+2);

      float n34 = Noise(Xint+2, Yint+2);

      //find the noise values of the four corners
      //corners, left-right, center
      float x0y0 = 0.0625f*(n01+n02+n03+n04) + 0.125f*(n05+n06+n07+n08) + 0.25f*(n09); 
      //left-right-(left-right-under 2 lines), (left-right-under 1 line)-center-(center under 2 lines), center under 1 line
      float x1y0 = 0.0625f*(n07+n12+n08+n14) + 0.125f*(n09+n16+n02+n04) + 0.25f*(n06);  
      float x0y1 = 0.0625f*(n05+n06+n23+n24) + 0.125f*(n03+n04+n09+n28) + 0.25f*(n08);  
      float x1y1 = 0.0625f*(n09+n16+n28+n34) + 0.125f*(n08+n14+n06+n24) + 0.25f*(n04);  

      //interpolate between those values according to the x and y fractions
      float v1 = Interpolate(x0y0, x1y0, Xfrac); //interpolate in x direction (y)
      float v2 = Interpolate(x0y1, x1y1, Xfrac); //interpolate in x direction (y+1)
      float fin = Interpolate(v1, v2, Yfrac);  //interpolate in y direction

      return fin;
    }

    float Interpolate(float x, float y, float a) const {
      return smoothstep(x, y, a);
      //return smootherstep(x, y, a);

      /*
      // NOTE: Equivalent to smoothstep function

      float negA = 1.0f - a;
      float negASqr = negA * negA;
      float fac1 = 3.0f * (negASqr) - 2.0f * (negASqr * negA);
      float aSqr = a * a;
      float fac2 = 3.0f * aSqr - 2.0f * (aSqr * a);

      return x * fac1 + y * fac2; //add the weighted factors
      */
    }

    // floating point numbers between -1.0 and 1.0
    float Noise(int x, int y) const {
      int n = x + y * 57;
      n = (n << 13) ^ n;
      int t = (n * (n * n * 15731 + 789221) + 1376312589) & 0x7fffffff;
      return 1.0f - float(t) * 0.931322574615478515625e-9f;
    }
  };
}