#ifndef ANIMATED_H
#define ANIMATED_H

#include <cassert>
#include <map>
#include <vector>

#include "ModelHeaders.h"
#include "MPQ.h"
#include "Quaternion.h"
#include "Vec3D.h"

//! \todo Pass this in somehow and don't define as extern.
// global time for global sequences
extern int globalTime;

namespace Animation
{
  namespace Interpolation
  {
    //! \todo C++0x: Change namespace to "enum class Type : int16_t", remove typedef.
    namespace Type
    {
      typedef int16_t Type_t;
      enum
      {
        NONE,
        LINEAR,
        HERMITE
      };
    }

    struct Linear
    {
      template<class AnimatedType>
      inline AnimatedType operator()( const float& percentage, const AnimatedType& start, const AnimatedType& end )
      {
        return start * ( 1.0f - percentage ) + end * percentage;
      }
    };

    //! \note "linear" interpolation for quaternions should be slerp by default.
    template<>
    inline Quaternion Linear::operator()( const float& percentage, const Quaternion& start, const Quaternion& end )
    {
      return Quaternion::slerp( percentage, start, end );
    }

    struct Hermite
    {
      template<class AnimatedType>
      inline AnimatedType operator()( const float& percentage, const AnimatedType& start, const AnimatedType& end, const AnimatedType& in, const AnimatedType& out )
      {
        const float percentage_2 = percentage * percentage;
        const float percentage_3 = percentage_2 * percentage;
        const float _2_percentage_3 = 2.0f * percentage_3;
        const float _3_percentage_2 = 3.0f * percentage_2;

        const float h1 = _2_percentage_3 - _3_percentage_2 + 1.0f;
        const float h2 = _3_percentage_2 - _2_percentage_3;
        const float h3 = percentage_3 - 2.0f * percentage_2 + percentage;
        const float h4 = percentage_3 - percentage_2;

        return AnimatedType( start * h1 + end * h2 + in * h3 + out * h4 );
      }
    };
  };
  
  template<class FROM, class TO>
  struct Conversion
  {
    inline TO operator()( const FROM& value )
    {
      return TO( value );
    }
  };
  
  template<>
  inline Quaternion Conversion<PackedQuaternion, Quaternion>::operator()( const PackedQuaternion& value )
  {
    //! \todo Check if this is really correct.
    return Quaternion(
      static_cast<float>( ( value.x > 0 ? value.x - 32767 : value.x + 32767 ) / 32767.0f ), 
      static_cast<float>( ( value.y > 0 ? value.y - 32767 : value.y + 32767 ) / 32767.0f ),
      static_cast<float>( ( value.z > 0 ? value.z - 32767 : value.z + 32767 ) / 32767.0f ),
      static_cast<float>( ( value.w > 0 ? value.w - 32767 : value.w + 32767 ) / 32767.0f ) );
  }
  
  template<>
  inline float Conversion<int16_t, float>::operator()( const int16_t& value )
  {
    return value / 32767.0f;
  }
  
  //! \note AnimatedType is the type of data getting animated.
  //! \note DataType is the type of data stored.
  //! \note The conversion from DataType to AnimatedType is done via Animation::Conversion.
  template<class AnimatedType, class DataType = AnimatedType>
  class M2Value
  {
  private:
    typedef uint32_t TimestampType;
    typedef uint32_t AnimationIdType;
  
    typedef std::vector<AnimatedType> AnimatedTypeVectorType;
    typedef std::vector<TimestampType> TimestampTypeVectorType;
    
    Animation::Conversion<DataType, AnimatedType> _conversion;
  
    static const int32_t NO_GLOBAL_SEQUENCE = -1;
    int32_t _globalSequenceID;
    int32_t* _globalSequences;
  
    Animation::Interpolation::Type::Type_t _interpolationType;
  
    std::map<AnimationIdType, TimestampTypeVectorType> times;
    std::map<AnimationIdType, AnimatedTypeVectorType> data;
  
    // for nonlinear interpolations:
    std::map<AnimationIdType, AnimatedTypeVectorType> in;
    std::map<AnimationIdType, AnimatedTypeVectorType> out;
  
  public:
    bool uses( AnimationIdType anim )
    {
      if( _globalSequenceID != NO_GLOBAL_SEQUENCE )
      {
        anim = AnimationIdType();
      }
  
      return !data[anim].empty();
    }
  
    AnimatedType getValue( AnimationIdType anim, TimestampType time )
    {
      if( _globalSequenceID != NO_GLOBAL_SEQUENCE )
      {
        if( _globalSequences[_globalSequenceID] )
        {
          time = globalTime % _globalSequences[_globalSequenceID];
        }
        else
        {
          time = TimestampType();
        }
        anim = AnimationIdType();
      }
      
      TimestampTypeVectorType& timestampVector = times[anim];
      AnimatedTypeVectorType& dataVector = data[anim];
      AnimatedTypeVectorType& inVector = in[anim];
      AnimatedTypeVectorType& outVector = out[anim];
  
      if( dataVector.empty() )
      {
        return AnimatedType();
      }
  
      AnimatedType result = dataVector[0];
  
      if( !timestampVector.empty() )
      {
        TimestampType max_time = timestampVector.back();
        if( max_time > 0 )
        {
          time %= max_time;
        }
        else
        {
          time = TimestampType();
        }
  
        size_t pos = 0;
        for( size_t i = 0; i < timestampVector.size() - 1; ++i )
        {
          if( time >= timestampVector[i] && time < timestampVector[i+1] )
          {
            pos = i;
            break;
          }
        }
  
        TimestampType t1 = timestampVector[pos];
        TimestampType t2 = timestampVector[pos + 1];
        const float percentage = ( time - t1 ) / static_cast<float>( t2 - t1 );
  
        switch( _interpolationType )
        {
          case Animation::Interpolation::Type::NONE:
          {
            result = dataVector[pos];
          }
          break;
  
          case Animation::Interpolation::Type::LINEAR:
          {
            Animation::Interpolation::Linear interpolation;
            result = interpolation( percentage, dataVector[pos], dataVector[pos + 1] );
          }
          break;
  
          case Animation::Interpolation::Type::HERMITE:
          {
            Animation::Interpolation::Hermite interpolation;
            result = interpolation( percentage, dataVector[pos], dataVector[pos + 1], inVector[pos], outVector[pos] );
          }
          break;
        }
      }
  
      return result;
    }
  
    //! \todo Use a vector of MPQFile& for the anim files instead for safety.
    void init(const AnimationBlock& animationBlock, const MPQFile& file, int32_t* globalSequences, MPQFile** animfiles = NULL )
    {
      assert( animationBlock.nTimes == animationBlock.nKeys );
      
      _interpolationType = animationBlock.type;
      
      _globalSequences = globalSequences;
      _globalSequenceID = animationBlock.seq;
      if( _globalSequenceID != NO_GLOBAL_SEQUENCE )
      {
        assert( _globalSequences && "Animation said to have global sequence, but pointer to global sequence data is NULL" );
      }
  
      const AnimationBlockHeader* timestampHeaders = file.get<AnimationBlockHeader>( animationBlock.ofsTimes );
      const AnimationBlockHeader* keyHeaders = file.get<AnimationBlockHeader>( animationBlock.ofsKeys );
  
      for( size_t j = 0; j < animationBlock.nTimes; ++j )
      {
        const TimestampType* timestamps = animfiles && animfiles[j] ?
                                            animfiles[j]->get<TimestampType>( timestampHeaders[j].ofsEntries ) :
                                            file.get<TimestampType>( timestampHeaders[j].ofsEntries );
  
        for( size_t i = 0; i < timestampHeaders[j].nEntries; ++i )
        {
          times[j].push_back( timestamps[i] );
        }
      }
  
      for( size_t j = 0; j < animationBlock.nKeys; ++j )
      {
        const DataType* keys = animfiles && animfiles[j] ?
                                  animfiles[j]->get<DataType>( keyHeaders[j].ofsEntries ) :
                                  file.get<DataType>( keyHeaders[j].ofsEntries );
        
        switch( _interpolationType )
        {
          case Animation::Interpolation::Type::NONE:
          case Animation::Interpolation::Type::LINEAR:
            for( size_t i = 0; i < keyHeaders[j].nEntries; ++i )
            {
              data[j].push_back( _conversion( keys[i] ) );
            }
            break;
            
          case Animation::Interpolation::Type::HERMITE:
            for( size_t i = 0; i < keyHeaders[j].nEntries; ++i )
            {
              data[j].push_back( _conversion( keys[i * 3] ) );
              in[j].push_back( _conversion( keys[i * 3 + 1] ) );
              out[j].push_back( _conversion( keys[i * 3 + 2] ) );
            }
            break;
        }
      }
    }
  
    void apply( AnimatedType function( const AnimatedType ) )
    {
      switch( _interpolationType )
      {
        case Animation::Interpolation::Type::NONE:
        case Animation::Interpolation::Type::LINEAR:
          for( size_t i = 0; i < data.size(); ++i )
          {
            for( size_t j = 0; j < data[i].size(); ++j )
            {
              data[i][j] = function( data[i][j] );
            }
          }
          break;
  
        case Animation::Interpolation::Type::HERMITE:
          for( size_t i = 0; i < data.size(); ++i )
          {
            for( size_t j = 0; j < data[i].size(); ++j )
            {
              data[i][j] = function( data[i][j] );
              in[i][j] = function( in[i][j] );
              out[i][j] = function( out[i][j] );
            }
          }
          break;
      }
    }
  };
};

#endif
