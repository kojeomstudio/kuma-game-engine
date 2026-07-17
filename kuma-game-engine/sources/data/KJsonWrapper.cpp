#include "KJsonWrapper.h"

#include <cstdio>
#include <fstream>
#include <nlohmann/json.hpp>
#include <sstream>

namespace kuma::data
{

struct KJsonWrapper::FImpl
{
    nlohmann::json Json;
};

KJsonWrapper::KJsonWrapper() : Impl(new FImpl()) {}

KJsonWrapper::~KJsonWrapper()
{
    delete Impl;
}

KJsonWrapper::KJsonWrapper(const KJsonWrapper& Other) : Impl(new FImpl(*Other.Impl)) {}

KJsonWrapper& KJsonWrapper::operator=(const KJsonWrapper& Other)
{
    if (this != &Other)
    {
        delete Impl;
        Impl = new FImpl(*Other.Impl);
    }
    return *this;
}

KJsonWrapper::KJsonWrapper(KJsonWrapper&& Other) noexcept : Impl(Other.Impl)
{
    Other.Impl = nullptr;
}

KJsonWrapper& KJsonWrapper::operator=(KJsonWrapper&& Other) noexcept
{
    if (this != &Other)
    {
        delete Impl;
        Impl = Other.Impl;
        Other.Impl = nullptr;
    }
    return *this;
}

bool KJsonWrapper::LoadFromFile(const std::string& Path)
{
    std::ifstream FileStream(Path);
    if (!FileStream.is_open())
    {
        std::fprintf(stderr, "[KJsonWrapper] Failed to open file: %s\n", Path.c_str());
        return false;
    }

    try
    {
        Impl->Json = nlohmann::json::parse(FileStream);
        return true;
    }
    catch (const nlohmann::json::exception& Ex)
    {
        std::fprintf(stderr, "[KJsonWrapper] Parse error in %s: %s\n", Path.c_str(), Ex.what());
        return false;
    }
}

bool KJsonWrapper::LoadFromString(const std::string& Content)
{
    try
    {
        Impl->Json = nlohmann::json::parse(Content);
        return true;
    }
    catch (const nlohmann::json::exception& Ex)
    {
        std::fprintf(stderr, "[KJsonWrapper] Parse error: %s\n", Ex.what());
        return false;
    }
}

bool KJsonWrapper::SaveToFile(const std::string& Path, int Indent) const
{
    std::ofstream FileStream(Path);
    if (!FileStream.is_open())
    {
        std::fprintf(stderr, "[KJsonWrapper] Failed to write file: %s\n", Path.c_str());
        return false;
    }

    FileStream << Impl->Json.dump(Indent);
    return true;
}

bool KJsonWrapper::IsValid() const
{
    return Impl && !Impl->Json.is_null();
}

bool KJsonWrapper::HasKey(const std::string& Key) const
{
    return Impl->Json.is_object() && Impl->Json.contains(Key);
}

bool KJsonWrapper::GetBool(const std::string& Key, bool DefaultValue) const
{
    if (!HasKey(Key))
    {
        return DefaultValue;
    }
    return Impl->Json[Key].get<bool>();
}

int KJsonWrapper::GetInt(const std::string& Key, int DefaultValue) const
{
    if (!HasKey(Key))
    {
        return DefaultValue;
    }
    return Impl->Json[Key].get<int>();
}

float KJsonWrapper::GetFloat(const std::string& Key, float DefaultValue) const
{
    if (!HasKey(Key))
    {
        return DefaultValue;
    }
    return Impl->Json[Key].get<float>();
}

double KJsonWrapper::GetDouble(const std::string& Key, double DefaultValue) const
{
    if (!HasKey(Key))
    {
        return DefaultValue;
    }
    return Impl->Json[Key].get<double>();
}

std::string KJsonWrapper::GetString(const std::string& Key, const std::string& DefaultValue) const
{
    if (!HasKey(Key))
    {
        return DefaultValue;
    }
    return Impl->Json[Key].get<std::string>();
}

void KJsonWrapper::SetBool(const std::string& Key, bool Value)
{
    Impl->Json[Key] = Value;
}

void KJsonWrapper::SetInt(const std::string& Key, int Value)
{
    Impl->Json[Key] = Value;
}

void KJsonWrapper::SetFloat(const std::string& Key, float Value)
{
    Impl->Json[Key] = Value;
}

void KJsonWrapper::SetDouble(const std::string& Key, double Value)
{
    Impl->Json[Key] = Value;
}

void KJsonWrapper::SetString(const std::string& Key, const std::string& Value)
{
    Impl->Json[Key] = Value;
}

std::string KJsonWrapper::Dump(int Indent) const
{
    return Impl->Json.dump(Indent);
}

} // namespace kuma::data
