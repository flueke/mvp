#include <chrono>
#include <iostream>

#include "gtest/gtest.h"
#include "mvlc_command_builders.h"
#include "mvlc_constants.h"
#include "vme_constants.h"

using std::cout;
using std::endl;

using namespace mesytec::mvlc;
using SuperCT = SuperCommandType;
using StackCT = StackCommand::CommandType;

TEST(mvlc_commands, SuperReferenceWord)
{
    auto builder = SuperCommandBuilder().addReferenceWord(0x1337u);

    ASSERT_EQ(builder.getCommands().size(), 1u);
    ASSERT_EQ(builder.getCommands()[0].type, SuperCommandType::ReferenceWord);
    ASSERT_EQ(builder.getCommands()[0].value, 0x1337u);

    std::vector<u32> expected =
    {
        static_cast<u32>(SuperCT::CmdBufferStart) << super_commands::SuperCmdShift,
        (static_cast<u32>(SuperCT::ReferenceWord) << super_commands::SuperCmdShift) | 0x1337u,
        static_cast<u32>(SuperCT::CmdBufferEnd) << super_commands::SuperCmdShift,
    };

    auto buffer = make_command_buffer(builder);

    ASSERT_EQ(buffer, expected);
}

TEST(mvlc_commands, SuperReadLocal)
{
    auto builder = SuperCommandBuilder().addReadLocal(0x1337u);

    ASSERT_EQ(builder.getCommands().size(), 1u);
    ASSERT_EQ(builder.getCommands()[0].type, SuperCommandType::ReadLocal);
    ASSERT_EQ(builder.getCommands()[0].address, 0x1337u);
    ASSERT_EQ(builder.getCommands()[0].value, 0);

    std::vector<u32> expected =
    {
        static_cast<u32>(SuperCT::CmdBufferStart) << super_commands::SuperCmdShift,
        (static_cast<u32>(SuperCT::ReadLocal) << super_commands::SuperCmdShift) | 0x1337u,
        static_cast<u32>(SuperCT::CmdBufferEnd) << super_commands::SuperCmdShift,
    };

    auto buffer = make_command_buffer(builder);

    ASSERT_EQ(buffer, expected);
}

TEST(mvlc_commands, SuperReadLocalBlock)
{
    auto builder = SuperCommandBuilder().addReadLocalBlock(0x1337u, 42);

    ASSERT_EQ(builder.getCommands().size(), 1u);
    ASSERT_EQ(builder.getCommands()[0].type, SuperCommandType::ReadLocalBlock);
    ASSERT_EQ(builder.getCommands()[0].address, 0x1337u);
    ASSERT_EQ(builder.getCommands()[0].value, 42);

    std::vector<u32> expected =
    {
        static_cast<u32>(SuperCT::CmdBufferStart) << super_commands::SuperCmdShift,
        (static_cast<u32>(SuperCT::ReadLocalBlock) << super_commands::SuperCmdShift) | 0x1337u,
        42,
        static_cast<u32>(SuperCT::CmdBufferEnd) << super_commands::SuperCmdShift,
    };

    auto buffer = make_command_buffer(builder);

    ASSERT_EQ(buffer, expected);
}

TEST(mvlc_commands, SuperWriteLocal)
{
    auto builder = SuperCommandBuilder().addWriteLocal(0x1337u, 42);

    ASSERT_EQ(builder.getCommands().size(), 1u);
    ASSERT_EQ(builder.getCommands()[0].type, SuperCommandType::WriteLocal);
    ASSERT_EQ(builder.getCommands()[0].address, 0x1337u);
    ASSERT_EQ(builder.getCommands()[0].value, 42);

    std::vector<u32> expected =
    {
        static_cast<u32>(SuperCT::CmdBufferStart) << super_commands::SuperCmdShift,
        (static_cast<u32>(SuperCT::WriteLocal) << super_commands::SuperCmdShift) | 0x1337u,
        42,
        static_cast<u32>(SuperCT::CmdBufferEnd) << super_commands::SuperCmdShift,
    };

    auto buffer = make_command_buffer(builder);

    ASSERT_EQ(buffer, expected);
}

TEST(mvlc_commands, SuperWriteReset)
{
    auto builder = SuperCommandBuilder().addWriteReset();

    ASSERT_EQ(builder.getCommands().size(), 1u);
    ASSERT_EQ(builder.getCommands()[0].type, SuperCommandType::WriteReset);
    ASSERT_EQ(builder.getCommands()[0].address, 0);
    ASSERT_EQ(builder.getCommands()[0].value, 0);

    std::vector<u32> expected =
    {
        static_cast<u32>(SuperCT::CmdBufferStart) << super_commands::SuperCmdShift,
        (static_cast<u32>(SuperCT::WriteReset) << super_commands::SuperCmdShift),
        static_cast<u32>(SuperCT::CmdBufferEnd) << super_commands::SuperCmdShift,
    };

    auto buffer = make_command_buffer(builder);

    ASSERT_EQ(buffer, expected);
}

TEST(mvlc_commands, SuperAddCommands)
{
    auto builder = SuperCommandBuilder().addCommands(
        {
            { SuperCommandType::ReadLocal, 0x1337u, 0 },
            { SuperCommandType::WriteLocal, 0x1338u, 42 },
        });

    ASSERT_EQ(builder.getCommands().size(), 2u);
    ASSERT_EQ(builder.getCommands()[0].type, SuperCommandType::ReadLocal);
    ASSERT_EQ(builder.getCommands()[0].address, 0x1337u);
    ASSERT_EQ(builder.getCommands()[0].value, 0);

    ASSERT_EQ(builder.getCommands()[1].type, SuperCommandType::WriteLocal);
    ASSERT_EQ(builder.getCommands()[1].address, 0x1338u);
    ASSERT_EQ(builder.getCommands()[1].value, 42);

    std::vector<u32> expected =
    {
        static_cast<u32>(SuperCT::CmdBufferStart) << super_commands::SuperCmdShift,
        (static_cast<u32>(SuperCT::ReadLocal) << super_commands::SuperCmdShift) | 0x1337u,
        (static_cast<u32>(SuperCT::WriteLocal) << super_commands::SuperCmdShift) | 0x1338u,
        42,
        static_cast<u32>(SuperCT::CmdBufferEnd) << super_commands::SuperCmdShift,
    };

    auto buffer = make_command_buffer(builder);

    ASSERT_EQ(buffer, expected);
}

TEST(mvlc_commands, SuperVMERead)
{
    auto builder = SuperCommandBuilder().addVMERead(0x1337u, 0x09u, VMEDataWidth::D16);

    // Only checking that at least the WriteLocal commands for StackStart and
    // StackEnd have been added.
    auto commands = builder.getCommands();

    ASSERT_TRUE(commands.size() >= 2);

    ASSERT_EQ(commands.front().type, SuperCommandType::WriteLocal);
    ASSERT_EQ(commands.front().value >> stack_commands::CmdShift,
              static_cast<u32>(StackCommandType::StackStart));

    ASSERT_EQ(commands.back().type, SuperCommandType::WriteLocal);
    ASSERT_EQ(commands.back().value >> stack_commands::CmdShift,
              static_cast<u32>(StackCommandType::StackEnd));

    auto buffer = make_command_buffer(builder);

#if 0
    cout << "buffer size = " << buffer.size() << endl;

    for (u32 value: buffer)
    {
        cout << std::showbase << std::hex << value << endl;
    }
#endif
    ASSERT_EQ(buffer.size(), 10);

    ASSERT_EQ(buffer[0], static_cast<u32>(SuperCT::CmdBufferStart) << super_commands::SuperCmdShift);

    ASSERT_EQ(buffer[1], (static_cast<u32>(SuperCT::WriteLocal) << super_commands::SuperCmdShift) | (stacks::StackMemoryBegin + AddressIncrement * 0));
    ASSERT_EQ(buffer[2], static_cast<u32>(StackCommandType::StackStart) << stack_commands::CmdShift);

    ASSERT_EQ(buffer[3], (static_cast<u32>(SuperCT::WriteLocal) << super_commands::SuperCmdShift) | (stacks::StackMemoryBegin + AddressIncrement * 1));
    ASSERT_EQ(buffer[4], ((static_cast<u32>(StackCT::VMERead) << stack_commands::CmdShift)
                          | (0x09u << stack_commands::CmdArg0Shift)
                          | (static_cast<u32>(VMEDataWidth::D16) << stack_commands::CmdArg1Shift)));

    ASSERT_EQ(buffer[5], (static_cast<u32>(SuperCT::WriteLocal) << super_commands::SuperCmdShift) | (stacks::StackMemoryBegin + AddressIncrement * 2));
    ASSERT_EQ(buffer[6], 0x1337);

    ASSERT_EQ(buffer[7], (static_cast<u32>(SuperCT::WriteLocal) << super_commands::SuperCmdShift) | (stacks::StackMemoryBegin + AddressIncrement * 3));
    ASSERT_EQ(buffer[8], static_cast<u32>(StackCommandType::StackEnd) << stack_commands::CmdShift);

    ASSERT_EQ(buffer[9], static_cast<u32>(SuperCT::CmdBufferEnd) << super_commands::SuperCmdShift);
}

TEST(mvlc_commands, SuperFromBuffer)
{
    SuperCommandBuilder builder;
    builder.addReferenceWord(0xabcd);
    builder.addReadLocal(0x1337u);
    builder.addReadLocalBlock(0x1338u, 42);
    builder.addWriteLocal(0x1339u, 43);
    builder.addWriteReset();
    builder.addVMERead(0x6070, 0x09, VMEDataWidth::D16);
    builder.addVMEBlockRead(0x1234, vme_amods::BLT32, 44);
    builder.addVMEWrite(0x6070, 42, 0x09, VMEDataWidth::D32);

    auto buffer = make_command_buffer(builder);
    auto builder2 = super_builder_from_buffer(buffer);

    ASSERT_EQ(builder.getCommands(), builder2.getCommands());

#if 0
    cout << "buffer size = " << buffer.size() << endl;

    for (u32 value: buffer)
    {
        cout << std::showbase << std::hex << value << endl;
    }
#endif
}

TEST(mvlc_commands, StackVMERead)
{
    auto builder = StackCommandBuilder().addVMERead(0x1337u, 0x09u, VMEDataWidth::D32);
    auto commands = builder.getCommands();
    ASSERT_EQ(builder.getGroupCount(), 1);
    ASSERT_EQ(commands.size(), 1u);
    ASSERT_EQ(commands.front().type, StackCommand::CommandType::VMERead);
    ASSERT_EQ(commands.front().address, 0x1337u);
    ASSERT_EQ(commands.front().amod, 0x09u);
    ASSERT_EQ(commands.front().dataWidth, VMEDataWidth::D32);
}

TEST(mvlc_commands, StackVMEWrite)
{
    auto builder = StackCommandBuilder().addVMEWrite(0x1337u, 42u, 0x09u, VMEDataWidth::D32);
    auto commands = builder.getCommands();
    ASSERT_EQ(builder.getGroupCount(), 1);
    ASSERT_EQ(commands.size(), 1u);
    ASSERT_EQ(commands.front().type, StackCommand::CommandType::VMEWrite);
    ASSERT_EQ(commands.front().address, 0x1337u);
    ASSERT_EQ(commands.front().value, 42u);
    ASSERT_EQ(commands.front().amod, 0x09u);
    ASSERT_EQ(commands.front().dataWidth, VMEDataWidth::D32);
}

TEST(mvlc_commands, StackVMEBlockRead)
{
    auto builder = StackCommandBuilder().addVMEBlockRead(0x1337u, 0x09u, 111);
    auto commands = builder.getCommands();
    ASSERT_EQ(builder.getGroupCount(), 1);
    ASSERT_EQ(commands.size(), 1u);
    ASSERT_EQ(commands.front().type, StackCommand::CommandType::VMERead);
    ASSERT_EQ(commands.front().address, 0x1337u);
    ASSERT_EQ(commands.front().amod, 0x09u);
    ASSERT_EQ(commands.front().transfers, 111);
}

TEST(mvlc_commands, StackVMEBlockRead2eSST)
{
    auto builder = StackCommandBuilder().addVMEBlockRead(0x1337u, Blk2eSSTRate::Rate276MB, 222);
    auto commands = builder.getCommands();
    ASSERT_EQ(builder.getGroupCount(), 1);
    ASSERT_EQ(commands.size(), 1u);
    ASSERT_EQ(commands.front().type, StackCommand::CommandType::VMERead);
    ASSERT_EQ(commands.front().address, 0x1337u);
    ASSERT_EQ(commands.front().amod, vme_amods::Blk2eSST64);
    ASSERT_EQ(commands.front().rate, Blk2eSSTRate::Rate276MB);
    ASSERT_EQ(commands.front().transfers, 222);
}

TEST(mvlc_commands, StackVMEBlockReadSwapped)
{
    auto builder = StackCommandBuilder().addVMEBlockReadSwapped(0x1337u, 111);
    auto commands = builder.getCommands();
    ASSERT_EQ(builder.getGroupCount(), 1);
    ASSERT_EQ(commands.size(), 1u);
    ASSERT_EQ(commands.front().type, StackCommand::CommandType::VMEMBLTSwapped);
    ASSERT_EQ(commands.front().address, 0x1337u);
    ASSERT_EQ(commands.front().amod, 0x08u); // MBLT
    ASSERT_EQ(commands.front().transfers, 111);
}

TEST(mvlc_commands, StackVMEBlockRead2eSSTSwapped)
{
    auto builder = StackCommandBuilder().addVMEBlockReadSwapped(0x1337u, Blk2eSSTRate::Rate276MB, 222);
    auto commands = builder.getCommands();
    ASSERT_EQ(builder.getGroupCount(), 1);
    ASSERT_EQ(commands.size(), 1u);
    ASSERT_EQ(commands.front().type, StackCommand::CommandType::VMEMBLTSwapped);
    ASSERT_EQ(commands.front().address, 0x1337u);
    ASSERT_EQ(commands.front().amod, vme_amods::Blk2eSST64);
    ASSERT_EQ(commands.front().rate, Blk2eSSTRate::Rate276MB);
    ASSERT_EQ(commands.front().transfers, 222);
}

TEST(mvlc_commands, StackWriteMarker)
{
    auto builder = StackCommandBuilder().addWriteMarker(0x87654321u);
    auto commands = builder.getCommands();
    ASSERT_EQ(builder.getGroupCount(), 1);
    ASSERT_EQ(commands.size(), 1u);
    ASSERT_EQ(commands.front().type, StackCommand::CommandType::WriteMarker);
    ASSERT_EQ(commands.front().value, 0x87654321u);
}

TEST(mvlc_commands, StackCustomCommand)
{
    StackCommand custom = {};
    custom.type = StackCommand::CommandType::Custom;
    custom.transfers = 42; // number of output words the custom stack data produces

    const std::vector<u32> customData =
    {
        0x12345678u,
        0xdeadbeefu,
        0x87654321u,
        0xabcdefabu,
    };

    custom.customValues = customData;

    StackCommandBuilder builder;
    builder.addCommand(custom);

    auto commands = builder.getCommands();
    ASSERT_EQ(builder.getGroupCount(), 1);
    ASSERT_EQ(commands.size(), 1u);
    ASSERT_EQ(commands.front().type, StackCommand::CommandType::Custom);
    ASSERT_EQ(commands.front().transfers, 42);
    ASSERT_EQ(commands.front().customValues, customData);
}

// Note: this test fails when adding a custom command to the stack builder.
TEST(mvlc_commands, StackFromBuffer)
{
    StackCommandBuilder builder;

    ASSERT_EQ(builder.getGroupCount(), 0);
    builder.addVMERead(0x1337u, 0x09u, VMEDataWidth::D16);
    builder.addVMEBlockRead(0x1338u, vme_amods::BLT32, 42);
    builder.addVMEWrite(0x1339u, 43, 0x09u, VMEDataWidth::D32);
    builder.addWriteMarker(0x87654321u);
    builder.addSetAddressIncMode(AddressIncrementMode::Memory);
    builder.addWait(42069);
    builder.addSignalAccu();
    builder.addMaskShiftAccu(0x0FF0, 7);
    builder.addSetAccu(1234);
    builder.addReadToAccu(0x1340u, 0x09u, VMEDataWidth::D32);
    builder.addCompareLoopAccu(AccuComparator::GT, 9000u);


    ASSERT_EQ(builder.getGroupCount(), 1);

    // Test cmd -> string -> cmd conversion
    for (const auto &cmd: builder.getCommands())
    {
        auto cmdString = to_string(cmd);
        //cout << cmdString << endl;
        auto cmdParsed = stack_command_from_string(cmdString);
        //if (cmd != cmdParsed)
        //    cout << "foo" << std::endl;
        ASSERT_EQ(cmd, cmdParsed);
    }

    // Test cmd -> buffer -> cmd conversion
    for (const auto &cmd: builder.getCommands())
    {
        auto buffer = make_stack_buffer(cmd);
        auto bufcmds = stack_commands_from_buffer(buffer);
        ASSERT_EQ(bufcmds.size(), 1u);
        auto cmdParsed = bufcmds[0];
        ASSERT_EQ(cmd, cmdParsed);
    }

    // Convert the whole builder to a stack buffer, then parse the buffer back
    // into a builder and finally compare both builders.
    auto buffer = make_stack_buffer(builder);
    auto builder2 = stack_builder_from_buffer(buffer);

    ASSERT_EQ(builder.getCommands(), builder2.getCommands());
}

TEST(mvlc_commands, StackGroups)
{
    StackCommandBuilder builder;

    ASSERT_EQ(builder.getGroupCount(), 0);

    builder.beginGroup("first");
    builder.addVMERead(0x1337u, 0x09u, VMEDataWidth::D16);

    ASSERT_EQ(builder.getGroupCount(), 1);
    ASSERT_EQ(builder.getGroup(0).name, "first");
    ASSERT_EQ(builder.getGroup("first").name, "first");
    ASSERT_EQ(builder.getCommands(0)[0].type, StackCT::VMERead);
    ASSERT_EQ(builder.getCommands(0)[0].address, 0x1337u);

    builder.beginGroup("second");
    builder.addVMEWrite(0x1338u, 42, 0x09u, VMEDataWidth::D32);

    ASSERT_EQ(builder.getGroupCount(), 2);
    ASSERT_EQ(builder.getGroup(0).name, "first");
    ASSERT_EQ(builder.getGroup("first").name, "first");
    ASSERT_EQ(builder.getCommands(0)[0].type, StackCT::VMERead);
    ASSERT_EQ(builder.getCommands(0)[0].address, 0x1337u);

    ASSERT_EQ(builder.getGroup(1).name, "second");
    ASSERT_EQ(builder.getGroup("second").name, "second");
    ASSERT_EQ(builder.getCommands(1)[0].type, StackCT::VMEWrite);
    ASSERT_EQ(builder.getCommands(1)[0].address, 0x1338u);

    // String lookups do not return a group ref but a copy of the group.
    // Looking up a nonexistent group should yield an empty group result.
    ASSERT_TRUE(builder.getGroup("noexistent").name.empty());
    ASSERT_TRUE(builder.getGroup("noexistent").commands.empty());
}

TEST(mvlc_commands, StackCommandToString)
{
    StackCommandBuilder builder;
    builder.addVMERead(0x1337u, 0x09u, VMEDataWidth::D16);
    builder.addVMEBlockRead(0x1338u, vme_amods::BLT32, 42);
    builder.addVMEWrite(0x1339u, 43, 0x09u, VMEDataWidth::D32);
    builder.addWriteMarker(0x87654321u);
    builder.addSoftwareDelay(std::chrono::milliseconds(100));

    StackCommand custom = {};
    custom.type = StackCommand::CommandType::Custom;
    custom.transfers = 42; // number of output words the custom stack data produces
    custom.customValues =
    {
        0x12345678u,
        0xdeadbeefu,
        0x87654321u,
        0xabcdefabu,
    };

    builder.addCommand(custom);

    for (const auto &cmd: builder.getCommands())
    {
        auto cmdString = to_string(cmd);
        cout << cmdString << endl;

        auto cmdParsed = stack_command_from_string(cmdString);

        ASSERT_EQ(cmd, cmdParsed);
    }
}
