#include <Windows.h>
#include <winioctl.h>

#include <cstdint>
#include <string>
#include <vector>

#if !defined(_WIN32) && !defined(_WIN64)
#error This class is only supported on windows!
#endif#

namespace Apfl
{
	class WinHandle
	{

	private:
		HANDLE m_Handle;

	public:
		WinHandle() : m_Handle(nullptr) { }
		WinHandle(const WinHandle& Handle) : m_Handle(Handle.get()) { }
		WinHandle(HANDLE Handle) : m_Handle(Handle) { }

		~WinHandle()
		{
			if (m_Handle != nullptr && m_Handle != INVALID_HANDLE_VALUE)
				CloseHandle(m_Handle);
		}

		WinHandle& operator=(WinHandle& Handle) { m_Handle = Handle.get(); return *this; }
		WinHandle& operator=(HANDLE Handle) { m_Handle = Handle; return *this; }

		bool operator==(const WinHandle& Handle) { return Handle.get() == m_Handle; }
		bool operator==(HANDLE Handle) { return Handle == m_Handle; }

		bool operator!=(const WinHandle& Handle) { return Handle.get() != m_Handle; }
		bool operator!=(HANDLE Handle) { return Handle != m_Handle; }

		HANDLE get() { return m_Handle; }
		const HANDLE get() const { return m_Handle; }

		HANDLE set(HANDLE Handle) { m_Handle = Handle; return m_Handle; }

	};
}

namespace Apfl
{
	template<class StringType> class _SecureStringBlock : public StringType
	{

	public:
		~_SecureStringBlock()
		{
			std::memset(const_cast<char*>(StringType::data()), 0x00, StringType::size());
			StringType::clear();
		}

	};

	// for compile-time string encryption
	template<class StringType, std::size_t StringLength> class _SecureString
	{

	private:
		static constexpr int32_t const_atoi(char c) { return static_cast<int32_t>(c - '0'); }

	private:
		using value_type = typename StringType::value_type;
		static constexpr std::size_t m_StringEnd = StringLength - 1;

		// use compile time as xor key
		static constexpr auto m_XorKey = static_cast<value_type>(
			_SecureString::const_atoi(__TIME__[7]) +
			_SecureString::const_atoi(__TIME__[6]) * 10 +
			_SecureString::const_atoi(__TIME__[4]) * 60 +
			_SecureString::const_atoi(__TIME__[3]) * 600 +
			_SecureString::const_atoi(__TIME__[1]) * 3600 +
			_SecureString::const_atoi(__TIME__[0]) * 36000);

	private:
		value_type m_Buffer[StringLength];

	public:
		constexpr __forceinline _SecureString(value_type const (&String)[StringLength])
			: _SecureString(String, std::make_index_sequence<m_StringEnd>()) { }

		inline _SecureStringBlock<StringType> str() const
		{
			_SecureStringBlock<StringType> str;

			str.resize(StringLength);
			std::copy(m_Buffer, m_Buffer + m_StringEnd, const_cast<value_type*>(str.data()));

			for (std::size_t i = 0; i != m_StringEnd; ++i)
				str[i] = _SecureString::process(str[i], i);

			return str;
		}

		inline operator value_type() const { return _SecureString::str(); }

	private:
		template<std::size_t... Indices>
		constexpr __forceinline _SecureString(value_type const (&String)[StringLength], std::index_sequence<Indices...>)
			: m_Buffer{ _SecureString::process(String[Indices], Indices)..., '\0' } { }

		static __forceinline constexpr auto process(value_type c, std::size_t iter) { return static_cast<value_type>(c ^ (m_XorKey + iter)); }

	};

	// ---------------------------------------------------------------- //
	template<std::size_t StringLength>
	using _SecureStringA = _SecureString<std::string, StringLength>;

	template<std::size_t StringLength>
	using _SecureStringW = _SecureString<std::wstring, StringLength>;

	template<std::size_t StringLength>
	using _SecureStringU16 = _SecureString<std::u16string, StringLength>;

	template<std::size_t StringLength>
	using _SecureStringU32 = _SecureString<std::u32string, StringLength>;
	// ---------------------------------------------------------------- //

	template<typename StringType, std::size_t StringLength_1, std::size_t StringLength_2>
	inline auto operator==(const _SecureString<StringType, StringLength_1>& lhs, const _SecureString<StringType, StringLength_2>& rhs)
	{
		return StringLength_1 == StringLength_2 && lhs.str() == rhs.str();
	}

	template<typename StringType, std::size_t StringLength>
	inline auto operator==(const StringType& lhs, const _SecureString<StringType, StringLength>& rhs)
	{
		return lhs.size() == StringLength && lhs == rhs.str();
	}

	template<typename StreamType, typename StringType, std::size_t StringLength>
	inline StreamType& operator<<(StreamType& lhs, const _SecureString<StringType, StringLength>& rhs)
	{
		lhs << rhs.str(); return lhs;
	}

	template<typename StringType, std::size_t StringLength_1, std::size_t StringLength_2>
	inline auto operator+(const _SecureString<StringType, StringLength_1>& lhs, const _SecureString<StringType, StringLength_2>& rhs)
	{
		return lhs.str() + rhs.str();
	}

	template<typename StringType, std::size_t StringLength>
	inline auto operator+(const StringType& lhs, const _SecureString<StringType, StringLength>& rhs)
	{
		return lhs + rhs.str();
	}

	template<std::size_t StringLength>
	constexpr __forceinline auto SecureString(char const (&String)[StringLength]) { return _SecureStringA<StringLength>(String); }

	template<std::size_t StringLength>
	constexpr __forceinline auto SecureString(wchar_t const (&String)[StringLength]) { return _SecureStringW<StringLength>(String); }

	template<std::size_t StringLength>
	constexpr __forceinline auto SecureString(char16_t const (&String)[StringLength]) { return _SecureStringU16<StringLength>(String); }

	template<std::size_t StringLength>
	constexpr __forceinline auto SecureString(char32_t const (&String)[StringLength]) { return _SecureStringU32<StringLength>(String); }

}

struct sHWID
{
    std::string MainDiskSerial;
};

bool getDiskSerial(sHWID& Info)
{
    Apfl::WinHandle hDrive(CreateFileA(Apfl::SecureString("\\\\.\\PhysicalDrive0").str().c_str(), 0, FILE_SHARE_READ, nullptr, OPEN_EXISTING, 0, nullptr));
    if (hDrive == INVALID_HANDLE_VALUE) return false;

    STORAGE_PROPERTY_QUERY query;
    memset(&query, 0x00, sizeof(query));
    query.QueryType = STORAGE_QUERY_TYPE::PropertyStandardQuery;
    query.PropertyId = STORAGE_PROPERTY_ID::StorageDeviceProperty;

    std::vector<uint8_t> desc_buf(sizeof(STORAGE_DEVICE_DESCRIPTOR), 0x00);

    DWORD bytes_returned = 0;
    if (!DeviceIoControl(hDrive.get(), IOCTL_STORAGE_QUERY_PROPERTY, &query, sizeof(query), desc_buf.data(), sizeof(desc_buf), &bytes_returned, nullptr))
        return false;

    desc_buf.resize(reinterpret_cast<PSTORAGE_DEVICE_DESCRIPTOR>(desc_buf.data())->Size);
    if (!DeviceIoControl(hDrive.get(), IOCTL_STORAGE_QUERY_PROPERTY, &query, sizeof(query), desc_buf.data(), desc_buf.size(), &bytes_returned, nullptr))
        return false;

    PSTORAGE_DEVICE_DESCRIPTOR device_desc = reinterpret_cast<PSTORAGE_DEVICE_DESCRIPTOR>(desc_buf.data());
    if (device_desc->SerialNumberOffset != 0 && device_desc->SerialNumberOffset < desc_buf.size())
    {
        Info.MainDiskSerial = reinterpret_cast<LPCSTR>(device_desc) + device_desc->SerialNumberOffset;
        return true;
    }

    return false;
}