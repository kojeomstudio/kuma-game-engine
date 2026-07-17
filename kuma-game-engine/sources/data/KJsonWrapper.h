#pragma once

#include <string>

namespace kuma::data
{

class KJsonWrapper
{
public:
    KJsonWrapper();
    ~KJsonWrapper();

    KJsonWrapper(const KJsonWrapper&);
    KJsonWrapper& operator=(const KJsonWrapper&);
    KJsonWrapper(KJsonWrapper&&) noexcept;
    KJsonWrapper& operator=(KJsonWrapper&&) noexcept;

    bool LoadFromFile(const std::string& Path);
    bool LoadFromString(const std::string& Content);

    bool SaveToFile(const std::string& Path, int Indent = 4) const;

    bool IsValid() const;
    bool HasKey(const std::string& Key) const;

    bool GetBool(const std::string& Key, bool DefaultValue = false) const;
    int GetInt(const std::string& Key, int DefaultValue = 0) const;
    float GetFloat(const std::string& Key, float DefaultValue = 0.0f) const;
    double GetDouble(const std::string& Key, double DefaultValue = 0.0) const;
    std::string GetString(const std::string& Key, const std::string& DefaultValue = "") const;

    void SetBool(const std::string& Key, bool Value);
    void SetInt(const std::string& Key, int Value);
    void SetFloat(const std::string& Key, float Value);
    void SetDouble(const std::string& Key, double Value);
    void SetString(const std::string& Key, const std::string& Value);

    std::string Dump(int Indent = -1) const;

private:
    struct FImpl;
    FImpl* Impl = nullptr;
};

} // namespace kuma::data
