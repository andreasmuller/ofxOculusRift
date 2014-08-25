/************************************************************************************

PublicHeader:   OVR.h
Filename    :   OVR_Profile.h
Content     :   Structs and functions for loading and storing device profile settings
Created     :   February 14, 2013
Notes       :
   Profiles are used to store per-user settings that can be transferred and used
   across multiple applications.  For example, player IPD can be configured once 
   and reused for a unified experience across games.  Configuration and saving of profiles
   can be accomplished in game via the Profile API or by the official Oculus Configuration
   Utility.

Copyright   :   Copyright 2013 Oculus VR, Inc. All Rights reserved.

Licensed under the Oculus VR SDK License Version 2.0 (the "License"); 
you may not use the Oculus VR SDK except in compliance with the License, 
which is provided at the time of installation or download, or which 
otherwise accompanies this software in either electronic or hard copy form.

You may obtain a copy of the License at

http://www.oculusvr.com/licenses/LICENSE-2.0 

Unless required by applicable law or agreed to in writing, the Oculus VR SDK 
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

************************************************************************************/

#ifndef OVR_Profile_h
#define OVR_Profile_h

#include "Kernel/OVR_String.h"
#include "Kernel/OVR_RefCount.h"
#include "Kernel/OVR_Array.h"

namespace OVR {

// Defines the profile object for each device type
enum ProfileType
{
    Profile_Unknown       = 0,
    Profile_GenericHMD    = 10,
    Profile_RiftDK1       = 11,
    Profile_RiftDKHD      = 12,
};

class Profile;

// -----------------------------------------------------------------------------
// ***** ProfileManager

// Profiles are interfaced through a ProfileManager object.  Applications should
// create a ProfileManager each time they intend to read or write user profile data.
// The scope of the ProfileManager object defines when disk I/O is performed.  Disk
// reads are performed on the first profile access and disk writes are performed when
// the ProfileManager goes out of scope.  All profile interactions between these times
// are performed in local memory and are fast.  A typical profile interaction might
// look like this:
//
// {
//     Ptr<ProfileManager> pm      = *ProfileManager::Create();
//     Ptr<Profile>        profile = pm->LoadProfile(Profile_RiftDK1,
//                                                   pm->GetDefaultProfileName(Profile_RiftDK1));
//     if (profile)
//     {   // Retrieve the current profile settings
//     }
// }   // Profile will be destroyed and any disk I/O completed when going out of scope

class ProfileManager : public RefCountBase<ProfileManager>
{
protected:
    // Synchronize ProfileManager access since it may be accessed from multiple threads,
    // as it's shared through DeviceManager.
    Lock                    ProfileLock;
    Array<Ptr<Profile> >    ProfileCache;
    ProfileType             CacheDevice;
    String                  DefaultProfile;
    bool                    Changed;
    char                    NameBuff[32];
    
public:
    static ProfileManager* Create();

    // Static interface functions
    int                 GetProfileCount(ProfileType device);
    const char*         GetProfileName(ProfileType device, unsigned int index);
    bool                HasProfile(ProfileType device, const char* name);
    Profile*            LoadProfile(ProfileType device, unsigned int index);
    Profile*            LoadProfile(ProfileType device, const char* name);
    Profile*            GetDeviceDefaultProfile(ProfileType device);
    const char*         GetDefaultProfileName(ProfileType device);
    bool                SetDefaultProfileName(ProfileType device, const char* name);
    bool                Save(const Profile* profile);
    bool                Delete(const Profile* profile);

protected:
    ProfileManager();
    ~ProfileManager();
    void                LoadCache(ProfileType device);
    void                SaveCache();
    void                ClearCache();
    Profile*            CreateProfileObject(const char* user,
                                            ProfileType device,
                                            const char** device_name);
};

//-------------------------------------------------------------------
// ***** Profile

// The base profile for all users.  This object is not created directly.
// Instead derived device objects provide add specific device members to 
// the base profile

class Profile : public RefCountBase<Profile>
{
public:
    enum { MaxNameLen    = 32 };

    enum GenderType
    {
        Gender_Unspecified  = 0,
        Gender_Male         = 1,
        Gender_Female       = 2
    };

    ProfileType          Type;              // The type of device profile
    char                 Name[MaxNameLen];  // The name given to this profile

protected:
    GenderType           Gender;            // The gender of the user
    float                PlayerHeight;      // The height of the user in meters
    float                IPD;               // Distance between eyes in meters

public:
    virtual Profile*     Clone() const = 0;

    // These are properties which are intrinsic to the user and affect scene setup
    GenderType           GetGender()                     { return Gender; };
    float                GetPlayerHeight()               { return PlayerHeight; };
    float                GetIPD()                        { return IPD; };
    float                GetEyeHeight();    
    
    void                 SetGender(GenderType gender)    { Gender = gender; };
    void                 SetPlayerHeight(float height)   { PlayerHeight = height; };
    void                 SetIPD(float ipd)               { IPD = ipd; };

protected:
    Profile(ProfileType type, const char* name);
    
    virtual bool         ParseProperty(const char* prop, const char* sval);
    
    friend class ProfileManager;
};

//-----------------------------------------------------------------------------
// ***** HMDProfile

// The generic HMD profile is used for properties that are common to all headsets
class HMDProfile : public Profile
{
protected:
    // FOV extents in pixels measured by a user
    int                 LL;       // left eye outer extent
    int                 LR;       // left eye inner extent
    int                 RL;       // right eye inner extent
    int                 RR;       // right eye outer extent

public:
    virtual Profile*    Clone() const;

    void SetLL(int val) { LL = val; };
    void SetLR(int val) { LR = val; };
    void SetRL(int val) { RL = val; };
    void SetRR(int val) { RR = val; };

    int GetLL() { return LL; };
    int GetLR() { return LR; };
    int GetRL() { return RL; };
    int GetRR() { return RR; };

protected:
    HMDProfile(ProfileType type, const char* name);

    virtual bool        ParseProperty(const char* prop, const char* sval);

    friend class ProfileManager;
};

// For headsets that use eye cups
enum EyeCupType
{
    EyeCup_A = 0,
    EyeCup_B = 1,
    EyeCup_C = 2
};

//-----------------------------------------------------------------------------
// ***** RiftDK1Profile

// This profile is specific to the Rift Dev Kit 1 and contains overrides specific 
// to that device and lens cup settings.
class RiftDK1Profile : public HMDProfile
{
protected:
    EyeCupType          EyeCups;   // Which eye cup does the player use

public:
    virtual Profile*    Clone() const;

    EyeCupType          GetEyeCup() { return EyeCups; };
    void                SetEyeCup(EyeCupType cup) { EyeCups = cup; };

protected:
    RiftDK1Profile(const char* name);

    virtual bool        ParseProperty(const char* prop, const char* sval);

    friend class ProfileManager;
};

//-----------------------------------------------------------------------------
// ***** RiftDKHDProfile

// This profile is specific to the Rift HD Dev Kit and contains overrides specific 
// to that device and lens cup settings.
class RiftDKHDProfile : public HMDProfile
{
protected:
    EyeCupType          EyeCups;   // Which eye cup does the player use

public:
    virtual Profile*    Clone() const;

    EyeCupType          GetEyeCup() { return EyeCups; };
    void                SetEyeCup(EyeCupType cup) { EyeCups = cup; };

protected:
    RiftDKHDProfile(const char* name);

    virtual bool        ParseProperty(const char* prop, const char* sval);

    friend class ProfileManager;
};


String GetBaseOVRPath(bool create_dir);

}

#endif // OVR_Profile_h