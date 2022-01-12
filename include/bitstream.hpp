/// \file
/// \brief This class allows you to write and read native types as a string of bits.  NetworkBitStream is used extensively throughout RakNet and is designed to be used by users as well.
///
/// This file is part of RakNet Copyright 2003 Kevin Jenkins.
///
/// Usage of RakNet is subject to the appropriate license agreement.
/// Creative Commons Licensees are subject to the
/// license found at
/// http://creativecommons.org/licenses/by-nc/2.5/
/// Single application licensees are subject to the license found at
/// http://www.rakkarsoft.com/SingleApplicationLicense.html
/// Custom license users are subject to the terms therein.
/// GPL license users are subject to the GNU General Public
/// License as published by the Free
/// Software Foundation; either version 2 of the License, or (at your
/// option) any later version.

#pragma once

#include "exports.hpp"
#include "gtaquat.hpp"
#include "types.hpp"
#include <assert.h>
#include <float.h>
#include <math.h>

#ifdef _MSC_VER
#pragma warning(push)
#endif

/// Given a number of bits, return how many bytes are needed to represent that.
#define BITS_TO_BYTES(x) (((x) + 7) >> 3)
#define BYTES_TO_BITS(x) ((x) << 3)

#define MAGNITUDE_EPSILON 0.00001f

struct INetwork;

/// Used for specifying bit stream data types
enum class NetworkBitStreamValueType {
    NONE,
    BIT, ///< bool
    UINT8, ///< uint8_t
    UINT16, ///< uint16_t
    UINT32, ///< uint32_t
    UINT64, ///< uint64_t
    INT8, ///< int8_t
    INT16, ///< int16_t
    INT32, ///< int32_t
    INT64, ///< int64_t
    FLOAT, ///< float
    DOUBLE, ///< double
    VEC2, ///< vector3
    VEC3, ///< vector3
    VEC4, ///< vector4
    VEC3_COMPRESSED, ///< vector3
    VEC3_SAMP, ///< vector3
    HP_ARMOR_COMPRESSED, ///< vector2
    DYNAMIC_LEN_STR_8, ///< StringView
    DYNAMIC_LEN_STR_16, ///< StringView
    DYNAMIC_LEN_STR_32, ///< StringView
    FIXED_LEN_ARR_CHAR, ///< Span<char>
    FIXED_LEN_ARR_UINT8, ///< Span<uint8_t>
    FIXED_LEN_ARR_UINT16, ///< Span<uint16_t>
    FIXED_LEN_ARR_UINT32, ///< Span<uint32_t>
    GTA_QUAT, ///< GTAQuat
    COMPRESSED_STR ///< NetworkString
};

/// Type used for storing UTF-8 strings to pass to the networks
template <size_t Size>
struct NetworkString {
    /// Copy constructor
    NetworkString<Size>(const NetworkString<Size>& other) = delete;
    /// Copy assignment
    NetworkString<Size>& operator=(const NetworkString<Size>& other) = delete;
    /// Move constructor
    NetworkString<Size>(NetworkString<Size>&& other) = delete;
    /// Move assignment
    NetworkString<Size>& operator=(NetworkString<Size>&& other) = delete;

    NetworkString<Size>& operator=(StringView data) {
        span = Span<const char>(data.data(), data.length());
        return *this;
    }

    /// Constructor for holding std::string data without copying it or freeing it
    /// @param string The std::string whose data to hold
    NetworkString(StringView str)
        : span(str.data(), str.length())
    {
    }

    NetworkString(char* data, size_t length) :
        span(data, length)
    {
    }

    NetworkString()
        : span(data)
    {
    }

    void setData(StringView data) {
        span = Span<const char>(data.data(), data.length());
    }

    /// Conversion operator for copying data to a std::string
    operator String() const
    {
        return String(span.data(), span.size());
    }

    operator StringView() const
    {
        return StringView(span.data(), span.size());
    }

    Span<const char> getData() const {
        return span;
    }

    Span<const char>& getData() {
        return span;
    }

private:
    StaticArray<char, Size> data;
    Span<const char> span;
};

/// Arbitrary size, just picking something likely to be larger than most packets
#define NETWORK_BITSTREAM_STACK_ALLOCATION_SIZE 256

class OMP_API NetworkBitStream {
public:
    /// Default Constructor
    NetworkBitStream();

    /// Create the bitstream, with some number of bytes to immediately allocate.
    /// There is no benefit to calling this, unless you know exactly how many bytes you need and it is greater than NETWORK_BITSTREAM_STACK_ALLOCATION_SIZE.
    /// In that case all it does is save you one or more realloc calls.
    /// \param[in] initialBytesToAllocate the number of bytes to pre-allocate.
    NetworkBitStream(int initialBytesToAllocate);

    /// Initialize the NetworkBitStream, immediately setting the data it contains to a predefined pointer.
    /// Set \a _copyData to true if you want to make an internal copy of the data you are passing. Set it to false to just save a pointer to the data.
    /// You shouldn't call Write functions with \a _copyData as false, as this will write to unallocated memory
    /// 99% of the time you will use this function to cast Packet::data to a bitstream for reading, in which case you should write something as follows:
    /// \code
    /// RakNet::NetworkBitStream bs(packet->data, packet->length, false);
    /// \endcode
    /// \param[in] _data An array of bytes.
    /// \param[in] lengthInBytes Size of the \a _data.
    /// \param[in] _copyData true or false to make a copy of \a _data or not.
    NetworkBitStream(unsigned char* _data, unsigned int lengthInBytes, bool _copyData);

    /// Destructor
    ~NetworkBitStream();

    /// Resets the bitstream for reuse.
    void reset(void);

    ///Sets the read pointer back to the beginning of your data.
    void resetReadPointer(void);

    /// Sets the write pointer back to the beginning of your data.
    void resetWritePointer(void);

public:
    template <NetworkBitStreamValueType NetworkType, typename T>
    inline void write(T data) = delete;

    template <>
    inline void write<NetworkBitStreamValueType::BIT>(bool data)
    {
        if (data)
            Write1();
        else
            Write0();
    }

    template <>
    inline void write<NetworkBitStreamValueType::UINT8>(unsigned data)
    {
        Write(uint8_t(data));
    }

    template <>
    inline void write<NetworkBitStreamValueType::UINT16>(unsigned data)
    {
        Write(uint16_t(data));
    }

    template <>
    inline void write<NetworkBitStreamValueType::UINT32>(unsigned data)
    {
        Write(uint32_t(data));
    }

    template <>
    inline void write<NetworkBitStreamValueType::UINT64>(uint64_t data)
    {
        Write(data);
    }

    template <>
    inline void write<NetworkBitStreamValueType::INT8>(int data)
    {
        Write(int8_t(data));
    }

    template <>
    inline void write<NetworkBitStreamValueType::INT16>(int data)
    {
        Write(int16_t(data));
    }

    template <>
    inline void write<NetworkBitStreamValueType::INT32>(int data)
    {
        Write(int32_t(data));
    }

    template <>
    inline void write<NetworkBitStreamValueType::INT64>(int64_t data)
    {
        Write(data);
    }

    template <>
    inline void write<NetworkBitStreamValueType::DOUBLE>(double data)
    {
        Write(data);
    }

    template <>
    inline void write<NetworkBitStreamValueType::FLOAT>(float data)
    {
        Write(data);
    }

    template <>
    inline void write<NetworkBitStreamValueType::VEC2>(Vector2 data)
    {
        Write(data);
    }

    template <>
    inline void write<NetworkBitStreamValueType::VEC3>(Vector3 data)
    {
        Write(data);
    }

    template <>
    inline void write<NetworkBitStreamValueType::VEC4>(Vector4 data)
    {
        Write(data);
    }

    template <>
    inline void write<NetworkBitStreamValueType::DYNAMIC_LEN_STR_8>(StringView data)
    {
        Write(static_cast<uint8_t>(data.length()));
        Write(data.data(), data.length());
    }

    template <>
    inline void write<NetworkBitStreamValueType::DYNAMIC_LEN_STR_16>(StringView data)
    {
        Write(static_cast<uint16_t>(data.length()));
        Write(data.data(), data.length());
    }

    template <>
    inline void write<NetworkBitStreamValueType::DYNAMIC_LEN_STR_32>(StringView data)
    {
        Write(static_cast<uint32_t>(data.length()));
        Write(data.data(), data.length());
    }

    template <>
    inline void write<NetworkBitStreamValueType::FIXED_LEN_ARR_CHAR>(Span<char> data)
    {
        Write(reinterpret_cast<const char*>(data.data()), data.length() * sizeof(char));
    }

    template <>
    inline void write<NetworkBitStreamValueType::FIXED_LEN_ARR_UINT8>(Span<uint8_t> data)
    {
        Write(reinterpret_cast<const char*>(data.data()), data.length() * sizeof(uint8_t));
    }

    template <>
    inline void write<NetworkBitStreamValueType::FIXED_LEN_ARR_UINT16>(Span<uint16_t> data)
    {
        Write(reinterpret_cast<const char*>(data.data()), data.length() * sizeof(uint16_t));
    }

    template <>
    inline void write<NetworkBitStreamValueType::FIXED_LEN_ARR_UINT32>(Span<uint32_t> data)
    {
        Write(reinterpret_cast<const char*>(data.data()), data.length() * sizeof(uint32_t));
    }
    template <>
    inline void write<NetworkBitStreamValueType::HP_ARMOR_COMPRESSED>(Vector2 data)
    {
        uint8_t ha = (data.x >= 100 ? 0x0F : (uint8_t)CEILDIV((int)data.x, 7)) << 4 | (data.y >= 100 ? 0x0F : (uint8_t)CEILDIV((int)data.y, 7));
        Write(ha);
    }

    template <>
    inline void write<NetworkBitStreamValueType::VEC3_SAMP>(Vector3 data);

    template <>
    inline void write<NetworkBitStreamValueType::GTA_QUAT>(GTAQuat data)
    {
        WriteNormQuat(data.q.w, data.q.x, data.q.y, data.q.z);
    }

    template <>
    inline void write<NetworkBitStreamValueType::COMPRESSED_STR>(StringView data)
    {
        WriteCompressedStr(data);
    }

    void WriteCompressedStr(StringView data);

    template <NetworkBitStreamValueType NetworkType, typename T>
    inline bool read(T& data) = delete;

    template <>
    inline bool read<NetworkBitStreamValueType::BIT>(bool& data);

    template <>
    inline bool read<NetworkBitStreamValueType::UINT8>(unsigned& data)
    {
        uint8_t tmp;
        const bool res = Read(tmp);
        data = tmp;
        return res;
    }

    template <>
    inline bool read<NetworkBitStreamValueType::UINT16>(unsigned& data)
    {
        uint16_t tmp;
        const bool res = Read(tmp);
        data = tmp;
        return res;
    }

    template <>
    inline bool read<NetworkBitStreamValueType::UINT32>(unsigned& data)
    {
        uint32_t tmp;
        const bool res = Read(tmp);
        data = tmp;
        return res;
    }

    template <>
    inline bool read<NetworkBitStreamValueType::UINT64>(uint64_t& data)
    {
        return Read(data);
    }

    template <>
    inline bool read<NetworkBitStreamValueType::INT8>(int& data)
    {
        int8_t tmp;
        const bool res = Read(tmp);
        data = tmp;
        return res;
    }

    template <>
    inline bool read<NetworkBitStreamValueType::INT16>(int& data)
    {
        int16_t tmp;
        const bool res = Read(tmp);
        data = tmp;
        return res;
    }

    template <>
    inline bool read<NetworkBitStreamValueType::INT32>(int& data)
    {
        int32_t tmp;
        const bool res = Read(tmp);
        data = tmp;
        return res;
    }

    template <>
    inline bool read<NetworkBitStreamValueType::INT64>(int64_t& data)
    {
        return Read(data);
    }

    template <>
    inline bool read<NetworkBitStreamValueType::DOUBLE>(double& data)
    {
        return Read(data);
    }

    template <>
    inline bool read<NetworkBitStreamValueType::FLOAT>(float& data)
    {
        return Read(data);
    }

    template <>
    inline bool read<NetworkBitStreamValueType::VEC2>(Vector2& data)
    {
        return Read(data);
    }

    template <>
    inline bool read<NetworkBitStreamValueType::VEC3>(Vector3& data)
    {
        return Read(data);
    }

    template <>
    inline bool read<NetworkBitStreamValueType::VEC4>(Vector4& data)
    {
        return Read(data);
    }

    template <>
    inline bool read<NetworkBitStreamValueType::DYNAMIC_LEN_STR_8>(Span<char>& data)
    {
        return readDynamicStr<uint8_t>(data);
    }

    template <>
    inline bool read<NetworkBitStreamValueType::DYNAMIC_LEN_STR_16>(Span<char>& data)
    {
        return readDynamicStr<uint16_t>(data);
    }

    template <>
    inline bool read<NetworkBitStreamValueType::DYNAMIC_LEN_STR_32>(Span<char>& data)
    {
        return readDynamicStr<uint32_t>(data);
    }

    template <>
    inline bool read<NetworkBitStreamValueType::FIXED_LEN_ARR_CHAR>(Span<char>& data)
    {
        return readFixedArray(data);
    }

    template <>
    inline bool read<NetworkBitStreamValueType::FIXED_LEN_ARR_UINT8>(Span<uint8_t>& data)
    {
        return readFixedArray(data);
    }

    template <>
    inline bool read<NetworkBitStreamValueType::FIXED_LEN_ARR_UINT16>(Span<uint16_t>& data)
    {
        return readFixedArray(data);
    }

    template <>
    inline bool read<NetworkBitStreamValueType::FIXED_LEN_ARR_UINT32>(Span<uint32_t>& data)
    {
        return readFixedArray(data);
    }

    template <>
    inline bool read<NetworkBitStreamValueType::HP_ARMOR_COMPRESSED>(Vector2& data)
    {
        uint8_t
            health,
            armour;
        if (!Read(health)) {
            return false;
        }
        if (!Read(armour)) {
            return false;
        }
        data = Vector2(health, armour);
        return true;
    }

    template <>
    inline bool read<NetworkBitStreamValueType::GTA_QUAT>(GTAQuat& data)
    {
        return Read(data);
    }

    template <typename LenType>
    bool readDynamicStr(Span<char>& data)
    {
        LenType len;
        if (!Read(len)) {
            return false;
        }
        if (len > unsigned(BITS_TO_BYTES(GetNumberOfUnreadBits()))) {
            return false;
        }
        len = len > data.length() ? data.length() : len;

        bool res = Read(data.data(), len);
        if (res) {
            data = Span<char>(data.data(), len);
        }
        return res;
    }

    template <typename T>
    bool readFixedArray(Span<T> data)
    {
        if (data.size() * sizeof(T) > unsigned(BITS_TO_BYTES(GetNumberOfUnreadBits()))) {
            return false;
        }

        return Read(reinterpret_cast<char*>(data.data()), data.size() * sizeof(T));
    }

    /// Bidirectional serialize/deserialize any integral type to/from a bitstream.  Undefine __BITSTREAM_NATIVE_END if you need endian swapping.
    /// \param[in] writeToBitstream true to write from your data to this bitstream.  False to read from this bitstream and write to your data
    /// \param[in] var The value to write
    /// \return true if \a writeToBitstream is true.  true if \a writeToBitstream is false and the read was successful.  false if \a writeToBitstream is false and the read was not successful.
    template <class templateType>
    bool Serialize(bool writeToBitstream, templateType& var);

    /// Bidirectional serialize/deserialize any integral type to/from a bitstream.  If the current value is different from the last value
    /// the current value will be written.  Otherwise, a single bit will be written
    /// \param[in] writeToBitstream true to write from your data to this bitstream.  False to read from this bitstream and write to your data
    /// \param[in] currentValue The current value to write
    /// \param[in] lastValue The last value to compare against.  Only used if \a writeToBitstream is true.
    /// \return true if \a writeToBitstream is true.  true if \a writeToBitstream is false and the read was successful.  false if \a writeToBitstream is false and the read was not successful.
    template <class templateType>
    bool SerializeDelta(bool writeToBitstream, templateType& currentValue, templateType lastValue);

    /// Bidirectional version of SerializeDelta when you don't know what the last value is, or there is no last value.
    /// \param[in] writeToBitstream true to write from your data to this bitstream.  False to read from this bitstream and write to your data
    /// \param[in] currentValue The current value to write
    /// \return true if \a writeToBitstream is true.  true if \a writeToBitstream is false and the read was successful.  false if \a writeToBitstream is false and the read was not successful.
    template <class templateType>
    bool SerializeDelta(bool writeToBitstream, templateType& currentValue);

    /// Bidirectional serialize/deserialize any integral type to/from a bitstream.  Undefine __BITSTREAM_NATIVE_END if you need endian swapping.
    /// If you are not using __BITSTREAM_NATIVE_END the opposite is true for types larger than 1 byte
    /// For floating point, this is lossy, using 2 bytes for a float and 4 for a double.  The range must be between -1 and +1.
    /// For non-floating point, this is lossless, but only has benefit if you use less than half the range of the type
    /// \param[in] writeToBitstream true to write from your data to this bitstream.  False to read from this bitstream and write to your data
    /// \param[in] var The value to write
    /// \return true if \a writeToBitstream is true.  true if \a writeToBitstream is false and the read was successful.  false if \a writeToBitstream is false and the read was not successful.
    template <class templateType>
    bool SerializeCompressed(bool writeToBitstream, templateType& var);

    /// Bidirectional serialize/deserialize any integral type to/from a bitstream.  If the current value is different from the last value
    /// the current value will be written.  Otherwise, a single bit will be written
    /// For floating point, this is lossy, using 2 bytes for a float and 4 for a double.  The range must be between -1 and +1.
    /// For non-floating point, this is lossless, but only has benefit if you use less than half the range of the type
    /// If you are not using __BITSTREAM_NATIVE_END the opposite is true for types larger than 1 byte
    /// \param[in] writeToBitstream true to write from your data to this bitstream.  False to read from this bitstream and write to your data
    /// \param[in] currentValue The current value to write
    /// \param[in] lastValue The last value to compare against.  Only used if \a writeToBitstream is true.
    /// \return true if \a writeToBitstream is true.  true if \a writeToBitstream is false and the read was successful.  false if \a writeToBitstream is false and the read was not successful.
    template <class templateType>
    bool SerializeCompressedDelta(bool writeToBitstream, templateType& currentValue, templateType lastValue);

    /// Save as SerializeCompressedDelta(templateType &currentValue, templateType lastValue) when we have an unknown second parameter
    template <class templateType>
    bool SerializeCompressedDelta(bool writeToBitstream, templateType& currentValue);

    /// Bidirectional serialize/deserialize an array or casted stream or raw data.  This does NOT do endian swapping.
    /// \param[in] writeToBitstream true to write from your data to this bitstream.  False to read from this bitstream and write to your data
    /// \param[in] input a byte buffer
    /// \param[in] numberOfBytes the size of \a input in bytes
    /// \return true if \a writeToBitstream is true.  true if \a writeToBitstream is false and the read was successful.  false if \a writeToBitstream is false and the read was not successful.
    bool Serialize(bool writeToBitstream, char* input, const int numberOfBytes);

    /// Bidirectional serialize/deserialize a normalized 3D vector, using (at most) 4 bytes + 3 bits instead of 12-24 bytes.  Will further compress y or z axis aligned vectors.
    /// Accurate to 1/32767.5.
    /// \param[in] writeToBitstream true to write from your data to this bitstream.  False to read from this bitstream and write to your data
    /// \param[in] x x
    /// \param[in] y y
    /// \param[in] z z
    /// \return true if \a writeToBitstream is true.  true if \a writeToBitstream is false and the read was successful.  false if \a writeToBitstream is false and the read was not successful.
    template <class templateType> // templateType for this function must be a float or double
    bool SerializeNormVector(bool writeToBitstream, templateType& x, templateType& y, templateType& z);

    /// Bidirectional serialize/deserialize a vector, using 10 bytes instead of 12.
    /// Loses accuracy to about 3/10ths and only saves 2 bytes, so only use if accuracy is not important.
    /// \param[in] writeToBitstream true to write from your data to this bitstream.  False to read from this bitstream and write to your data
    /// \param[in] x x
    /// \param[in] y y
    /// \param[in] z z
    /// \return true if \a writeToBitstream is true.  true if \a writeToBitstream is false and the read was successful.  false if \a writeToBitstream is false and the read was not successful.
    template <class templateType> // templateType for this function must be a float or double
    bool SerializeVector(bool writeToBitstream, templateType& x, templateType& y, templateType& z);

    /// Bidirectional serialize/deserialize a normalized quaternion in 6 bytes + 4 bits instead of 16 bytes.  Slightly lossy.
    /// \param[in] writeToBitstream true to write from your data to this bitstream.  False to read from this bitstream and write to your data
    /// \param[in] w w
    /// \param[in] x x
    /// \param[in] y y
    /// \param[in] z z
    /// \return true if \a writeToBitstream is true.  true if \a writeToBitstream is false and the read was successful.  false if \a writeToBitstream is false and the read was not successful.
    template <class templateType> // templateType for this function must be a float or double
    bool SerializeNormQuat(bool writeToBitstream, templateType& w, templateType& x, templateType& y, templateType& z);

    /// Bidirectional serialize/deserialize an orthogonal matrix by creating a quaternion, and writing 3 components of the quaternion in 2 bytes each
    /// for 6 bytes instead of 36
    /// Lossy, although the result is renormalized
    template <class templateType> // templateType for this function must be a float or double
    bool SerializeOrthMatrix(
        bool writeToBitstream,
        templateType& m00, templateType& m01, templateType& m02,
        templateType& m10, templateType& m11, templateType& m12,
        templateType& m20, templateType& m21, templateType& m22);

    /// Bidirectional serialize/deserialize numberToSerialize bits to/from the input. Right aligned
    /// data means in the case of a partial byte, the bits are aligned
    /// from the right (bit 0) rather than the left (as in the normal
    /// internal representation) You would set this to true when
    /// writing user data, and false when copying bitstream data, such
    /// as writing one bitstream to another
    /// \param[in] writeToBitstream true to write from your data to this bitstream.  False to read from this bitstream and write to your data
    /// \param[in] input The data
    /// \param[in] numberOfBitsToSerialize The number of bits to write
    /// \param[in] rightAlignedBits if true data will be right aligned
    /// \return true if \a writeToBitstream is true.  true if \a writeToBitstream is false and the read was successful.  false if \a writeToBitstream is false and the read was not successful.
    bool SerializeBits(bool writeToBitstream, unsigned char* input, int numberOfBitsToSerialize, const bool rightAlignedBits = true);

    /// Write any integral type to a bitstream.  Undefine __BITSTREAM_NATIVE_END if you need endian swapping.
    /// \param[in] var The value to write
    template <class templateType>
    void Write(templateType var);

    /// Write any integral type to a bitstream.  If the current value is different from the last value
    /// the current value will be written.  Otherwise, a single bit will be written
    /// \param[in] currentValue The current value to write
    /// \param[in] lastValue The last value to compare against
    template <class templateType>
    void WriteDelta(templateType currentValue, templateType lastValue);

    /// WriteDelta when you don't know what the last value is, or there is no last value.
    /// \param[in] currentValue The current value to write
    template <class templateType>
    void WriteDelta(templateType currentValue);

    /// Write any integral type to a bitstream.  Undefine __BITSTREAM_NATIVE_END if you need endian swapping.
    /// If you are not using __BITSTREAM_NATIVE_END the opposite is true for types larger than 1 byte
    /// For floating point, this is lossy, using 2 bytes for a float and 4 for a double.  The range must be between -1 and +1.
    /// For non-floating point, this is lossless, but only has benefit if you use less than half the range of the type
    /// \param[in] var The value to write
    template <class templateType>
    void WriteCompressed(templateType var);

    /// Write any integral type to a bitstream.  If the current value is different from the last value
    /// the current value will be written.  Otherwise, a single bit will be written
    /// For floating point, this is lossy, using 2 bytes for a float and 4 for a double.  The range must be between -1 and +1.
    /// For non-floating point, this is lossless, but only has benefit if you use less than half the range of the type
    /// If you are not using __BITSTREAM_NATIVE_END the opposite is true for types larger than 1 byte
    /// \param[in] currentValue The current value to write
    /// \param[in] lastValue The last value to compare against
    template <class templateType>
    void WriteCompressedDelta(templateType currentValue, templateType lastValue);

    /// Save as WriteCompressedDelta(templateType currentValue, templateType lastValue) when we have an unknown second parameter
    template <class templateType>
    void WriteCompressedDelta(templateType currentValue);

    /// Read any integral type from a bitstream.  Define __BITSTREAM_NATIVE_END if you need endian swapping.
    /// \param[in] var The value to read
    template <class templateType>
    bool Read(templateType& var);

    /// Read any integral type from a bitstream.  If the written value differed from the value compared against in the write function,
    /// var will be updated.  Otherwise it will retain the current value.
    /// ReadDelta is only valid from a previous call to WriteDelta
    /// \param[in] var The value to read
    template <class templateType>
    bool ReadDelta(templateType& var);

    /// Read any integral type from a bitstream.  Undefine __BITSTREAM_NATIVE_END if you need endian swapping.
    /// For floating point, this is lossy, using 2 bytes for a float and 4 for a double.  The range must be between -1 and +1.
    /// For non-floating point, this is lossless, but only has benefit if you use less than half the range of the type
    /// If you are not using __BITSTREAM_NATIVE_END the opposite is true for types larger than 1 byte
    /// \param[in] var The value to read
    template <class templateType>
    bool ReadCompressed(templateType& var);

    /// Read any integral type from a bitstream.  If the written value differed from the value compared against in the write function,
    /// var will be updated.  Otherwise it will retain the current value.
    /// the current value will be updated.
    /// For floating point, this is lossy, using 2 bytes for a float and 4 for a double.  The range must be between -1 and +1.
    /// For non-floating point, this is lossless, but only has benefit if you use less than half the range of the type
    /// If you are not using __BITSTREAM_NATIVE_END the opposite is true for types larger than 1 byte
    /// ReadCompressedDelta is only valid from a previous call to WriteDelta
    /// \param[in] var The value to read
    template <class templateType>
    bool ReadCompressedDelta(templateType& var);

    /// Write an array or casted stream or raw data.  This does NOT do endian swapping.
    /// \param[in] input a byte buffer
    /// \param[in] numberOfBytes the size of \a input in bytes
    void Write(const char* input, const int numberOfBytes);

    /// Write one bitstream to another
    /// \param[in] numberOfBits bits to write
    /// \param bitStream the bitstream to copy from
    void Write(NetworkBitStream* bitStream, int numberOfBits);
    void Write(NetworkBitStream* bitStream);

    /// Read a normalized 3D vector, using (at most) 4 bytes + 3 bits instead of 12-24 bytes.  Will further compress y or z axis aligned vectors.
    /// Accurate to 1/32767.5.
    /// \param[in] x x
    /// \param[in] y y
    /// \param[in] z z
    template <class templateType> // templateType for this function must be a float or double
    void WriteNormVector(templateType x, templateType y, templateType z);

    /// Write a vector, using 10 bytes instead of 12.
    /// Loses accuracy to about 3/10ths and only saves 2 bytes, so only use if accuracy is not important.
    /// \param[in] x x
    /// \param[in] y y
    /// \param[in] z z
    template <class templateType> // templateType for this function must be a float or double
    void WriteVector(templateType x, templateType y, templateType z);

    /// Write a normalized quaternion in 6 bytes + 4 bits instead of 16 bytes.  Slightly lossy.
    /// \param[in] w w
    /// \param[in] x x
    /// \param[in] y y
    /// \param[in] z z
    template <class templateType> // templateType for this function must be a float or double
    void WriteNormQuat(templateType w, templateType x, templateType y, templateType z);

    /// Write an orthogonal matrix by creating a quaternion, and writing 3 components of the quaternion in 2 bytes each
    /// for 6 bytes instead of 36
    /// Lossy, although the result is renormalized
    template <class templateType> // templateType for this function must be a float or double
    void WriteOrthMatrix(
        templateType m00, templateType m01, templateType m02,
        templateType m10, templateType m11, templateType m12,
        templateType m20, templateType m21, templateType m22);

    /// Read an array or casted stream of byte. The array
    /// is raw data. There is no automatic endian conversion with this function
    /// \param[in] output The result byte array. It should be larger than @em numberOfBytes.
    /// \param[in] numberOfBytes The number of byte to read
    /// \return true on success false if there is some missing bytes.
    bool Read(char* output, const int numberOfBytes);

    /// Read a normalized 3D vector, using (at most) 4 bytes + 3 bits instead of 12-24 bytes.  Will further compress y or z axis aligned vectors.
    /// Accurate to 1/32767.5.
    /// \param[in] x x
    /// \param[in] y y
    /// \param[in] z z
    template <class templateType> // templateType for this function must be a float or double
    bool ReadNormVector(templateType& x, templateType& y, templateType& z);

    /// Read 3 floats or doubles, using 10 bytes, where those float or doubles comprise a vector
    /// Loses accuracy to about 3/10ths and only saves 2 bytes, so only use if accuracy is not important.
    /// \param[in] x x
    /// \param[in] y y
    /// \param[in] z z
    template <class templateType> // templateType for this function must be a float or double
    bool ReadVector(templateType& x, templateType& y, templateType& z);

    /// Read a normalized quaternion in 6 bytes + 4 bits instead of 16 bytes.
    /// \param[in] w w
    /// \param[in] x x
    /// \param[in] y y
    /// \param[in] z z
    template <class templateType> // templateType for this function must be a float or double
    bool ReadNormQuat(templateType& w, templateType& x, templateType& y, templateType& z);

    /// Read an orthogonal matrix from a quaternion, reading 3 components of the quaternion in 2 bytes each and extrapolatig the 4th.
    /// for 6 bytes instead of 36
    /// Lossy, although the result is renormalized
    template <class templateType> // templateType for this function must be a float or double
    bool ReadOrthMatrix(
        templateType& m00, templateType& m01, templateType& m02,
        templateType& m10, templateType& m11, templateType& m12,
        templateType& m20, templateType& m21, templateType& m22);

    ///This is good to call when you are done with the stream to make
    /// sure you didn't leave any data left over void
    void AssertStreamEmpty(void);

    /// printf the bits in the stream.  Great for debugging.
    void PrintBits(void) const;

    /// Ignore data we don't intend to read
    /// \param[in] numberOfBits The number of bits to ignore
    void IgnoreBits(const int numberOfBits);

    ///Move the write pointer to a position on the array.
    /// \param[in] offset the offset from the start of the array.
    /// \attention
    /// Dangerous if you don't know what you are doing!
    /// For efficiency reasons you can only write mid-stream if your data is byte aligned.
    void SetWriteOffset(const int offset);

    /// Returns the length in bits of the stream
    inline int GetNumberOfBitsUsed(void) const { return GetWriteOffset(); }
    inline int GetWriteOffset(void) const { return numberOfBitsUsed; }

    ///Returns the length in bytes of the stream
    inline int GetNumberOfBytesUsed(void) const { return BITS_TO_BYTES(numberOfBitsUsed); }

    ///Returns the number of bits into the stream that we have read
    inline int GetReadOffset(void) const { return readOffset; }

    // Sets the read bit index
    inline void SetReadOffset(int newReadOffset) { readOffset = newReadOffset; }

    ///Returns the number of bits left in the stream that haven't been read
    inline int GetNumberOfUnreadBits(void) const { return readOffset > numberOfBitsUsed ? 0 : numberOfBitsUsed - readOffset; }

    /// Makes a copy of the internal data for you \a _data will point to
    /// the stream. Returns the length in bits of the stream. Partial
    /// bytes are left aligned
    /// \param[out] _data The allocated copy of GetData()
    int CopyData(unsigned char** _data) const;

    /// Set the stream to some initial data.
    /// \internal
    void SetData(unsigned char* input);

    /// Gets the data that NetworkBitStream is writing to / reading from
    /// Partial bytes are left aligned.
    /// \return A pointer to the internal state
    inline unsigned char* GetData(void) const { return data; }

    /// Write numberToWrite bits from the input source Right aligned
    /// data means in the case of a partial byte, the bits are aligned
    /// from the right (bit 0) rather than the left (as in the normal
    /// internal representation) You would set this to true when
    /// writing user data, and false when copying bitstream data, such
    /// as writing one bitstream to another
    /// \param[in] input The data
    /// \param[in] numberOfBitsToWrite The number of bits to write
    /// \param[in] rightAlignedBits if true data will be right aligned
    void WriteBits(const unsigned char* input, int numberOfBitsToWrite, const bool rightAlignedBits = true);

    /// Align the bitstream to the byte boundary and then write the
    /// specified number of bits.  This is faster than DoWriteBits but
    /// wastes the bits to do the alignment and requires you to call
    /// ReadAlignedBits at the corresponding read position.
    /// \param[in] input The data
    /// \param[in] numberOfBytesToWrite The size of data.
    void WriteAlignedBytes(const unsigned char* input, const int numberOfBytesToWrite);

    /// Read bits, starting at the next aligned bits. Note that the
    /// modulus 8 starting offset of the sequence must be the same as
    /// was used with DoWriteBits. This will be a problem with packet
    /// coalescence unless you byte align the coalesced packets.
    /// \param[in] output The byte array larger than @em numberOfBytesToRead
    /// \param[in] numberOfBytesToRead The number of byte to read from the internal state
    /// \return true if there is enough byte.
    bool ReadAlignedBytes(unsigned char* output, const int numberOfBytesToRead);

    /// Align the next write and/or read to a byte boundary.  This can
    /// be used to 'waste' bits to byte align for efficiency reasons It
    /// can also be used to force coalesced bitstreams to start on byte
    /// boundaries so so WriteAlignedBits and ReadAlignedBits both
    /// calculate the same offset when aligning.
    void AlignWriteToByteBoundary(void);

    /// Align the next write and/or read to a byte boundary.  This can
    /// be used to 'waste' bits to byte align for efficiency reasons It
    /// can also be used to force coalesced bitstreams to start on byte
    /// boundaries so so WriteAlignedBits and ReadAlignedBits both
    /// calculate the same offset when aligning.
    void AlignReadToByteBoundary(void);

    /// Read \a numberOfBitsToRead bits to the output source
    /// alignBitsToRight should be set to true to convert internal
    /// bitstream data to userdata. It should be false if you used
    /// DoWriteBits with rightAlignedBits false
    /// \param[in] output The resulting bits array
    /// \param[in] numberOfBitsToRead The number of bits to read
    /// \param[in] alignBitsToRight if true bits will be right aligned.
    /// \return true if there is enough bits to read
    bool ReadBits(unsigned char* output, int numberOfBitsToRead, const bool alignBitsToRight = true);

    /// Write a 0
    void Write0(void);

    /// Write a 1
    void Write1(void);

    /// Reads 1 bit and returns true if that bit is 1 and false if it is 0
    bool ReadBit(void);

    /// If we used the constructor version with copy data off, this
    /// *makes sure it is set to on and the data pointed to is copied.
    void AssertCopyData(void);

    /// Use this if you pass a pointer copy to the constructor
    /// *(_copyData==false) and want to overallocate to prevent
    /// *reallocation
    void SetNumberOfBitsAllocated(const unsigned int lengthInBits);

    /// Reallocates (if necessary) in preparation of writing numberOfBitsToWrite
    void AddBitsAndReallocate(const int numberOfBitsToWrite);

private:
    /// Assume the input source points to a native type, compress and write it.
    void WriteCompressed(const unsigned char* input, const int size, const bool unsignedData);

    /// Assume the input source points to a compressed native type. Decompress and read it.
    bool ReadCompressed(unsigned char* output, const int size, const bool unsignedData);

    int numberOfBitsUsed;

    int numberOfBitsAllocated;

    int readOffset;

    unsigned char* data;

    /// true if the internal buffer is copy of the data passed to the constructor
    bool copyData;

    /// BitStreams that use less than NETWORK_BITSTREAM_STACK_ALLOCATION_SIZE use the stack, rather than the heap to store data.  It switches over if NETWORK_BITSTREAM_STACK_ALLOCATION_SIZE is exceeded
    unsigned char stackData[NETWORK_BITSTREAM_STACK_ALLOCATION_SIZE];
};

template <class templateType>
inline bool NetworkBitStream::Serialize(bool writeToBitstream, templateType& var)
{
    if (writeToBitstream)
        Write(var);
    else
        return Read(var);
    return true;
}

template <class templateType>
inline bool NetworkBitStream::SerializeDelta(bool writeToBitstream, templateType& currentValue, templateType lastValue)
{
    if (writeToBitstream)
        WriteDelta(currentValue, lastValue);
    else
        return ReadDelta(currentValue);
    return true;
}

template <class templateType>
inline bool NetworkBitStream::SerializeDelta(bool writeToBitstream, templateType& currentValue)
{
    if (writeToBitstream)
        WriteDelta(currentValue);
    else
        return ReadDelta(currentValue);
    return true;
}

template <class templateType>
inline bool NetworkBitStream::SerializeCompressed(bool writeToBitstream, templateType& var)
{
    if (writeToBitstream)
        WriteCompressed(var);
    else
        return ReadCompressed(var);
    return true;
}

template <class templateType>
inline bool NetworkBitStream::SerializeCompressedDelta(bool writeToBitstream, templateType& currentValue, templateType lastValue)
{
    if (writeToBitstream)
        WriteCompressedDelta(currentValue, lastValue);
    else
        return ReadCompressedDelta(currentValue);
    return true;
}

template <class templateType>
inline bool NetworkBitStream::SerializeCompressedDelta(bool writeToBitstream, templateType& currentValue)
{
    if (writeToBitstream)
        WriteCompressedDelta(currentValue);
    else
        return ReadCompressedDelta(currentValue);
    return true;
}

inline bool NetworkBitStream::Serialize(bool writeToBitstream, char* input, const int numberOfBytes)
{
    if (writeToBitstream)
        Write(input, numberOfBytes);
    else
        return Read(input, numberOfBytes);
    return true;
}

template <class templateType>
inline bool NetworkBitStream::SerializeNormVector(bool writeToBitstream, templateType& x, templateType& y, templateType& z)
{
    if (writeToBitstream)
        WriteNormVector(x, y, z);
    else
        return ReadNormVector(x, y, z);
    return true;
}

template <class templateType>
inline bool NetworkBitStream::SerializeVector(bool writeToBitstream, templateType& x, templateType& y, templateType& z)
{
    if (writeToBitstream)
        WriteVector(x, y, z);
    else
        return ReadVector(x, y, z);
    return true;
}

template <class templateType>
inline bool NetworkBitStream::SerializeNormQuat(bool writeToBitstream, templateType& w, templateType& x, templateType& y, templateType& z)
{
    if (writeToBitstream)
        WriteNormQuat(w, x, y, z);
    else
        return ReadNormQuat(w, x, y, z);
    return true;
}

template <class templateType>
inline bool NetworkBitStream::SerializeOrthMatrix(
    bool writeToBitstream,
    templateType& m00, templateType& m01, templateType& m02,
    templateType& m10, templateType& m11, templateType& m12,
    templateType& m20, templateType& m21, templateType& m22)
{
    if (writeToBitstream)
        WriteOrthMatrix(m00, m01, m02, m10, m11, m12, m20, m21, m22);
    else
        return ReadOrthMatrix(m00, m01, m02, m10, m11, m12, m20, m21, m22);
    return true;
}

inline bool NetworkBitStream::SerializeBits(bool writeToBitstream, unsigned char* input, int numberOfBitsToSerialize, const bool rightAlignedBits)
{
    if (writeToBitstream)
        WriteBits(input, numberOfBitsToSerialize, rightAlignedBits);
    else
        return ReadBits(input, numberOfBitsToSerialize, rightAlignedBits);
    return true;
}

template <class templateType>
inline void NetworkBitStream::Write(templateType var)
{
#ifdef _MSC_VER
#pragma warning(disable : 4127) // conditional expression is constant
#endif
    WriteBits((unsigned char*)&var, sizeof(templateType) * 8, true);
}

/// Write any integral type to a bitstream.  If the current value is different from the last value
/// the current value will be written.  Otherwise, a single bit will be written
/// \param[in] currentValue The current value to write
/// \param[in] lastValue The last value to compare against
template <class templateType>
inline void NetworkBitStream::WriteDelta(templateType currentValue, templateType lastValue)
{
    if (currentValue == lastValue) {
        Write(false);
    } else {
        Write(true);
        Write(currentValue);
    }
}

/// Write a bool delta.  Same thing as just calling Write
/// \param[in] currentValue The current value to write
/// \param[in] lastValue The last value to compare against
template <>
inline void NetworkBitStream::WriteDelta(bool currentValue, bool lastValue)
{
#ifdef _MSC_VER
#pragma warning(disable : 4100) // warning C4100: 'lastValue' : unreferenced formal parameter
#endif
    Write(currentValue);
}

/// WriteDelta when you don't know what the last value is, or there is no last value.
/// \param[in] currentValue The current value to write
template <class templateType>
inline void NetworkBitStream::WriteDelta(templateType currentValue)
{
    Write(true);
    Write(currentValue);
}

/// Write any integral type to a bitstream.  Undefine __BITSTREAM_NATIVE_END if you need endian swapping.
/// For floating point, this is lossy, using 2 bytes for a float and 4 for a double.  The range must be between -1 and +1.
/// For non-floating point, this is lossless, but only has benefit if you use less than half the range of the type
/// If you are not using __BITSTREAM_NATIVE_END the opposite is true for types larger than 1 byte
/// \param[in] var The value to write
template <class templateType>
inline void NetworkBitStream::WriteCompressed(templateType var)
{
#ifdef _MSC_VER
#pragma warning(disable : 4127) // conditional expression is constant
#endif
    WriteCompressed((unsigned char*)&var, sizeof(templateType) * 8, true);
}

template <>
inline void NetworkBitStream::WriteCompressed(bool var)
{
    Write(var);
}

/// For values between -1 and 1
template <>
inline void NetworkBitStream::WriteCompressed(float var)
{
    //RakAssert(var > -1.01f && var < 1.01f);
    if (var < -1.0f)
        var = -1.0f;
    if (var > 1.0f)
        var = 1.0f;
    Write((unsigned short)((var + 1.0f) * 32767.5f));
}

/// For values between -1 and 1
template <>
inline void NetworkBitStream::WriteCompressed(double var)
{
    assert(var > -1.01 && var < 1.01);
    if (var < -1.0f)
        var = -1.0f;
    if (var > 1.0f)
        var = 1.0f;

    assert(sizeof(unsigned long) == 4);

    Write((unsigned long)((var + 1.0) * 2147483648.0));
}

/// Write any integral type to a bitstream.  If the current value is different from the last value
/// the current value will be written.  Otherwise, a single bit will be written
/// For floating point, this is lossy, using 2 bytes for a float and 4 for a double.  The range must be between -1 and +1.
/// For non-floating point, this is lossless, but only has benefit if you use less than half the range of the type
/// If you are not using __BITSTREAM_NATIVE_END the opposite is true for types larger than 1 byte
/// \param[in] currentValue The current value to write
/// \param[in] lastValue The last value to compare against
template <class templateType>
inline void NetworkBitStream::WriteCompressedDelta(templateType currentValue, templateType lastValue)
{
    if (currentValue == lastValue) {
        Write(false);
    } else {
        Write(true);
        WriteCompressed(currentValue);
    }
}

/// Write a bool delta.  Same thing as just calling Write
/// \param[in] currentValue The current value to write
/// \param[in] lastValue The last value to compare against
template <>
inline void NetworkBitStream::WriteCompressedDelta(bool currentValue, bool lastValue)
{
#ifdef _MSC_VER
#pragma warning(disable : 4100) // warning C4100: 'lastValue' : unreferenced formal parameter
#endif
    Write(currentValue);
}

/// Save as WriteCompressedDelta(templateType currentValue, templateType lastValue) when we have an unknown second parameter
template <class templateType>
inline void NetworkBitStream::WriteCompressedDelta(templateType currentValue)
{
    Write(true);
    WriteCompressed(currentValue);
}

/// Save as WriteCompressedDelta(bool currentValue, templateType lastValue) when we have an unknown second bool
template <>
inline void NetworkBitStream::WriteCompressedDelta(bool currentValue)
{
    Write(currentValue);
}

/// Read any integral type from a bitstream.  Define __BITSTREAM_NATIVE_END if you need endian swapping.
/// \param[in] var The value to read
template <class templateType>
inline bool NetworkBitStream::Read(templateType& var)
{
#ifdef _MSC_VER
#pragma warning(disable : 4127) // conditional expression is constant
#endif
    return ReadBits((unsigned char*)&var, sizeof(templateType) * 8, true);
}

/// Read a bool from a bitstream
/// \param[in] var The value to read
template <>
inline bool NetworkBitStream::Read(bool& var)
{
    if (GetNumberOfUnreadBits() == 0)
        return false;

    if (data[readOffset >> 3] & (0x80 >> (readOffset % 8))) // Is it faster to just write it out here?
        var = true;
    else
        var = false;

    ++readOffset;

    return true;
}

/// Read any integral type from a bitstream.  If the written value differed from the value compared against in the write function,
/// var will be updated.  Otherwise it will retain the current value.
/// ReadDelta is only valid from a previous call to WriteDelta
/// \param[in] var The value to read
template <class templateType>
inline bool NetworkBitStream::ReadDelta(templateType& var)
{
    bool dataWritten;
    bool success;
    success = Read(dataWritten);
    if (dataWritten)
        success = Read(var);
    return success;
}

/// Read a bool from a bitstream
/// \param[in] var The value to read
template <>
inline bool NetworkBitStream::ReadDelta(bool& var)
{
    return Read(var);
}

/// Read any integral type from a bitstream.  Undefine __BITSTREAM_NATIVE_END if you need endian swapping.
/// For floating point, this is lossy, using 2 bytes for a float and 4 for a double.  The range must be between -1 and +1.
/// For non-floating point, this is lossless, but only has benefit if you use less than half the range of the type
/// If you are not using __BITSTREAM_NATIVE_END the opposite is true for types larger than 1 byte
/// \param[in] var The value to read
template <class templateType>
inline bool NetworkBitStream::ReadCompressed(templateType& var)
{
#ifdef _MSC_VER
#pragma warning(disable : 4127) // conditional expression is constant
#endif
    return ReadCompressed((unsigned char*)&var, sizeof(templateType) * 8, true);
}

template <>
inline bool NetworkBitStream::ReadCompressed(bool& var)
{
    return Read(var);
}

/// For values between -1 and 1
template <>
inline bool NetworkBitStream::ReadCompressed(float& var)
{
    unsigned short compressedFloat;
    if (Read(compressedFloat)) {
        var = ((float)compressedFloat / 32767.5f - 1.0f);
        return true;
    }
    return false;
}

/// For values between -1 and 1
template <>
inline bool NetworkBitStream::ReadCompressed(double& var)
{
    unsigned long compressedFloat;
    if (Read(compressedFloat)) {
        var = ((double)compressedFloat / 2147483648.0 - 1.0);
        return true;
    }
    return false;
}

/// Read any integral type from a bitstream.  If the written value differed from the value compared against in the write function,
/// var will be updated.  Otherwise it will retain the current value.
/// the current value will be updated.
/// For floating point, this is lossy, using 2 bytes for a float and 4 for a double.  The range must be between -1 and +1.
/// For non-floating point, this is lossless, but only has benefit if you use less than half the range of the type
/// If you are not using __BITSTREAM_NATIVE_END the opposite is true for types larger than 1 byte
/// ReadCompressedDelta is only valid from a previous call to WriteDelta
/// \param[in] var The value to read
template <class templateType>
inline bool NetworkBitStream::ReadCompressedDelta(templateType& var)
{
    bool dataWritten;
    bool success;
    success = Read(dataWritten);
    if (dataWritten)
        success = ReadCompressed(var);
    return success;
}

/// Read a bool from a bitstream
/// \param[in] var The value to read
template <>
inline bool NetworkBitStream::ReadCompressedDelta(bool& var)
{
    return Read(var);
}

template <class templateType> // templateType for this function must be a float or double
void NetworkBitStream::WriteNormVector(templateType x, templateType y, templateType z)
{

    RakAssert(x <= 1.01 && y <= 1.01 && z <= 1.01 && x >= -1.01 && y >= -1.01 && z >= -1.01);

    if (x > 1.0)
        x = 1.0;
    if (y > 1.0)
        y = 1.0;
    if (z > 1.0)
        z = 1.0;
    if (x < -1.0)
        x = -1.0;
    if (y < -1.0)
        y = -1.0;
    if (z < -1.0)
        z = -1.0;

    Write((bool)(x < 0.0));
    if (y == 0.0)
        Write(true);
    else {
        Write(false);
        WriteCompressed((float)y);
        //Write((unsigned short)((y+1.0f)*32767.5f));
    }
    if (z == 0.0)
        Write(true);
    else {
        Write(false);
        WriteCompressed((float)z);
        //Write((unsigned short)((z+1.0f)*32767.5f));
    }
}

template <class templateType> // templateType for this function must be a float or double
void NetworkBitStream::WriteVector(templateType x, templateType y, templateType z)
{
    templateType magnitude = sqrt(x * x + y * y + z * z);
    Write((float)magnitude);
    if (magnitude > 0.0) {
        WriteCompressed((float)(x / magnitude));
        WriteCompressed((float)(y / magnitude));
        WriteCompressed((float)(z / magnitude));
        //	Write((unsigned short)((x/magnitude+1.0f)*32767.5f));
        //	Write((unsigned short)((y/magnitude+1.0f)*32767.5f));
        //	Write((unsigned short)((z/magnitude+1.0f)*32767.5f));
    }
}

template <class templateType> // templateType for this function must be a float or double
void NetworkBitStream::WriteNormQuat(templateType w, templateType x, templateType y, templateType z)
{
    Write((bool)(w < 0.0));
    Write((bool)(x < 0.0));
    Write((bool)(y < 0.0));
    Write((bool)(z < 0.0));
    Write((unsigned short)(fabs(x) * 65535.0));
    Write((unsigned short)(fabs(y) * 65535.0));
    Write((unsigned short)(fabs(z) * 65535.0));
    // Leave out w and calculate it on the target
}

template <class templateType> // templateType for this function must be a float or double
void NetworkBitStream::WriteOrthMatrix(
    templateType m00, templateType m01, templateType m02,
    templateType m10, templateType m11, templateType m12,
    templateType m20, templateType m21, templateType m22)
{
    double qw;
    double qx;
    double qy;
    double qz;

#ifdef _MSC_VER
#pragma warning(disable : 4100) // m10, m01 : unreferenced formal parameter
#endif

    // Convert matrix to quat
    // http://www.euclideanspace.com/maths/geometry/rotations/conversions/matrixToQuaternion/
    float sum;
    sum = 1 + m00 + m11 + m22;
    if (sum < 0.0f)
        sum = 0.0f;
    qw = sqrt(sum) / 2;
    sum = 1 + m00 - m11 - m22;
    if (sum < 0.0f)
        sum = 0.0f;
    qx = sqrt(sum) / 2;
    sum = 1 - m00 + m11 - m22;
    if (sum < 0.0f)
        sum = 0.0f;
    qy = sqrt(sum) / 2;
    sum = 1 - m00 - m11 + m22;
    if (sum < 0.0f)
        sum = 0.0f;
    qz = sqrt(sum) / 2;
    if (qw < 0.0)
        qw = 0.0;
    if (qx < 0.0)
        qx = 0.0;
    if (qy < 0.0)
        qy = 0.0;
    if (qz < 0.0)
        qz = 0.0;
    qx = _copysign(qx, m21 - m12);
    qy = _copysign(qy, m02 - m20);
    qz = _copysign(qz, m10 - m01);

    WriteNormQuat(qw, qx, qy, qz);
}

template <class templateType> // templateType for this function must be a float or double
bool NetworkBitStream::ReadNormVector(templateType& x, templateType& y, templateType& z)
{
    //	unsigned short sy, sz;
    bool yZero, zZero;
    bool xNeg;
    float cy, cz;

    Read(xNeg);

    Read(yZero);
    if (yZero)
        y = 0.0;
    else {
        ReadCompressed(cy);
        y = cy;
        //Read(sy);
        //y=((float)sy / 32767.5f - 1.0f);
    }

    if (!Read(zZero))
        return false;

    if (zZero)
        z = 0.0;
    else {
        //	if (!Read(sz))
        //		return false;

        //	z=((float)sz / 32767.5f - 1.0f);
        if (!ReadCompressed(cz))
            return false;
        z = cz;
    }

    x = (templateType)(sqrtf((templateType)1.0 - y * y - z * z));
    if (xNeg)
        x = -x;
    return true;
}

template <class templateType> // templateType for this function must be a float or double
bool NetworkBitStream::ReadVector(templateType& x, templateType& y, templateType& z)
{
    float magnitude;
    //unsigned short sx,sy,sz;
    if (!Read(magnitude))
        return false;
    if (magnitude != 0.0) {
        //	Read(sx);
        //	Read(sy);
        //	if (!Read(sz))
        //		return false;
        //	x=((float)sx / 32767.5f - 1.0f) * magnitude;
        //	y=((float)sy / 32767.5f - 1.0f) * magnitude;
        //	z=((float)sz / 32767.5f - 1.0f) * magnitude;
        float cx, cy, cz;
        ReadCompressed(cx);
        ReadCompressed(cy);
        if (!ReadCompressed(cz))
            return false;
        x = cx;
        y = cy;
        z = cz;
        x *= magnitude;
        y *= magnitude;
        z *= magnitude;
    } else {
        x = 0.0;
        y = 0.0;
        z = 0.0;
    }
    return true;
}

template <class templateType> // templateType for this function must be a float or double
bool NetworkBitStream::ReadNormQuat(templateType& w, templateType& x, templateType& y, templateType& z)
{
    bool cwNeg, cxNeg, cyNeg, czNeg;
    unsigned short cx, cy, cz;
    Read(cwNeg);
    Read(cxNeg);
    Read(cyNeg);
    Read(czNeg);
    Read(cx);
    Read(cy);
    if (!Read(cz))
        return false;

    // Calculate w from x,y,z
    x = (templateType)(cx / 65535.0);
    y = (templateType)(cy / 65535.0);
    z = (templateType)(cz / 65535.0);
    if (cxNeg)
        x = -x;
    if (cyNeg)
        y = -y;
    if (czNeg)
        z = -z;
    float difference = 1.0f - x * x - y * y - z * z;
    if (difference < 0.0f)
        difference = 0.0f;
    w = (templateType)(sqrt(difference));
    if (cwNeg)
        w = -w;
    return true;
}

template <class templateType> // templateType for this function must be a float or double
bool NetworkBitStream::ReadOrthMatrix(
    templateType& m00, templateType& m01, templateType& m02,
    templateType& m10, templateType& m11, templateType& m12,
    templateType& m20, templateType& m21, templateType& m22)
{
    float qw, qx, qy, qz;
    if (!ReadNormQuat(qw, qx, qy, qz))
        return false;

    // Quat to orthogonal rotation matrix
    // http://www.euclideanspace.com/maths/geometry/rotations/conversions/quaternionToMatrix/index.htm
    double sqw = (double)qw * (double)qw;
    double sqx = (double)qx * (double)qx;
    double sqy = (double)qy * (double)qy;
    double sqz = (double)qz * (double)qz;
    m00 = (templateType)(sqx - sqy - sqz + sqw); // since sqw + sqx + sqy + sqz =1
    m11 = (templateType)(-sqx + sqy - sqz + sqw);
    m22 = (templateType)(-sqx - sqy + sqz + sqw);

    double tmp1 = (double)qx * (double)qy;
    double tmp2 = (double)qz * (double)qw;
    m10 = (templateType)(2.0 * (tmp1 + tmp2));
    m01 = (templateType)(2.0 * (tmp1 - tmp2));

    tmp1 = (double)qx * (double)qz;
    tmp2 = (double)qy * (double)qw;
    m20 = (templateType)(2.0 * (tmp1 - tmp2));
    m02 = (templateType)(2.0 * (tmp1 + tmp2));
    tmp1 = (double)qy * (double)qz;
    tmp2 = (double)qx * (double)qw;
    m21 = (templateType)(2.0 * (tmp1 + tmp2));
    m12 = (templateType)(2.0 * (tmp1 - tmp2));

    return true;
}

template <>
inline void NetworkBitStream::write<NetworkBitStreamValueType::VEC3_SAMP>(Vector3 data)
{
    float magnitude = glm::length(data);
    Write(magnitude);
    if (magnitude > MAGNITUDE_EPSILON) {
        data /= magnitude;
        WriteCompressed(data.x);
        WriteCompressed(data.y);
        WriteCompressed(data.z);
    }
}

template <>
inline bool NetworkBitStream::read<NetworkBitStreamValueType::BIT>(bool& data)
{
    return Read(data);
}

#ifdef _MSC_VER
#pragma warning(pop)
#endif
