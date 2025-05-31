
#include <atlbase.h>
#include <appmodel.h>
#include <roapi.h>
#include <Windows.Management.Deployment.h>
#include <Windows.Foundation.Collections.h>
#include "hstring.hpp"
#include "appx.hpp"

#define CHECK_FAILURE_NORET(hr) \
    if (FAILED(hr))             \
        return;
std::vector<std::pair<std::wstring, std::wstring>> FindPackages(LPCWSTR checkid)
{
    std::vector<std::pair<std::wstring, std::wstring>> results;
    [&]()
    {
    CComPtr<ABI::Windows::Management::Deployment::IPackageManager> packageManager;
    CHECK_FAILURE_NORET(RoActivateInstance(AutoHString(RuntimeClass_Windows_Management_Deployment_PackageManager), reinterpret_cast<IInspectable**>(&packageManager)));

    CComPtr<ABI::Windows::Foundation::Collections::IIterable<ABI::Windows::ApplicationModel::Package*>> packagesFoundRaw;
    CHECK_FAILURE_NORET(packageManager->FindPackagesByUserSecurityId(AutoHString(L""), &packagesFoundRaw));

    CComPtr<ABI::Windows::Foundation::Collections::IIterator<ABI::Windows::ApplicationModel::Package*>> packagesFoundRaw_itor;
    CHECK_FAILURE_NORET(packagesFoundRaw->First(&packagesFoundRaw_itor));

    boolean fHasCurrent;
    CHECK_FAILURE_NORET(packagesFoundRaw_itor->get_HasCurrent(&fHasCurrent));
    while (fHasCurrent)
    {
        CComPtr<ABI::Windows::ApplicationModel::IPackage> package;
        CHECK_FAILURE_NORET(packagesFoundRaw_itor->get_Current(&package));
        CComPtr<ABI::Windows::ApplicationModel::IPackageId> packageid;
        CHECK_FAILURE_NORET(package->get_Id(&packageid));
        AutoHString strname;
        CHECK_FAILURE_NORET(packageid->get_Name(&strname));
        PCWSTR _name = strname;
        if (wcsstr(_name, checkid) == _name)
        {
            CComPtr<ABI::Windows::Storage::IStorageFolder> installpath;
            CHECK_FAILURE_NORET(package->get_InstalledLocation(&installpath));
            CComPtr<ABI::Windows::Storage::IStorageItem> item;
            CHECK_FAILURE_NORET(installpath.QueryInterface(&item));
            AutoHString str;
            item->get_Path(&str);
            results.push_back(std::make_pair(_name, (LPCWSTR)str));
        }
        CHECK_FAILURE_NORET(packagesFoundRaw_itor->MoveNext(&fHasCurrent));
    } }();
    return results;
}
