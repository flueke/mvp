#ifndef __MESYTEC_MVLC_MVLC_READOUT_CONFIG_H__
#define __MESYTEC_MVLC_MVLC_READOUT_CONFIG_H__

#include "mesytec-mvlc/mesytec-mvlc_export.h"

#include "mvlc_constants.h"
#include "mvlc_command_builders.h"

namespace mesytec
{
namespace mvlc
{

struct MESYTEC_MVLC_EXPORT CrateConfig
{
    /// How to connect to the MVLC, USB or ETH.
    ConnectionType connectionType;

    // For ConnectionType::USB: If usbIndex is >= 0 it takes precedence over a
    // non-empty usbSerial. If neither usbIndex nor usbSerial are set then a
    // connection to the first USB device is attempted.
    int usbIndex = -1;
    std::string usbSerial;

    // For ConnectionType::ETH: ethHost can be a hostname or an IPv4 address
    // string. If ethJumboEnable is set then 8k Ethernet frames will be used on
    // the data pipe.
    std::string ethHost;
    bool ethJumboEnable = false;

    // Per event readout command stacks.
    std::vector<StackCommandBuilder> stacks;

    // The trigger value for each of the command stacks.
    std::vector<u32> triggers;

    // Dedicated command list for initializing the trigger/io system.
    StackCommandBuilder initTriggerIO;

    // List of init commands to be run during DAQ startup.
    StackCommandBuilder initCommands;

    // List of stop commands to be run during DAQ shutdown.
    StackCommandBuilder stopCommands;

    // Multicast DAQ start commands
    StackCommandBuilder mcstDaqStart;

    // Multicast DAQ stop commands
    StackCommandBuilder mcstDaqStop;

    bool operator==(const CrateConfig &o) const;
    inline bool operator!=(const CrateConfig &o) const { return !(*this == o); }
};

// Cratecconfig serialization to/from YAML
std::string MESYTEC_MVLC_EXPORT to_yaml(const CrateConfig &crateConfig);
CrateConfig MESYTEC_MVLC_EXPORT crate_config_from_yaml(const std::string &yaml);
CrateConfig MESYTEC_MVLC_EXPORT crate_config_from_yaml(std::istream &input);
CrateConfig MESYTEC_MVLC_EXPORT crate_config_from_yaml_file(const std::string &filename);

// StackCommandBuilder serialization to/from YAML
std::string MESYTEC_MVLC_EXPORT to_yaml(const StackCommandBuilder &sb);
StackCommandBuilder MESYTEC_MVLC_EXPORT stack_command_builder_from_yaml(const std::string &yaml);
StackCommandBuilder MESYTEC_MVLC_EXPORT stack_command_builder_from_yaml(std::istream &input);
StackCommandBuilder MESYTEC_MVLC_EXPORT stack_command_builder_from_yaml_file(const std::string &filename);


} // end namespace mvlc
} // end namespace mesytec

#endif /* __MESYTEC_MVLC_MVLC_READOUT_CONFIG_H__ */
