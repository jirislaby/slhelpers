// SPDX-License-Identifier: GPL-2.0-only

#include <cassert>
#include <string>

#include "kerncvs/SupportedConf.h"

using namespace SlKernCVS;

int main()
{
	static const std::string supportedConf = {
		"#\n"
		"# comment comment\n"
		"-	drivers/input/joystick/*\n"
		"  drivers/mtd/spi-nor/spi-nor # comment\n"
		"- drivers/mtd/*\n"
		"- drivers/usb/typec/altmodes/typec_displayport.ko\n"
		"+ocfs2-kmp      fs/ocfs2/ocfs2_stackglue\n"
		"+base           drivers/net/ethernet/8390/8390\n"
		"-!optional      drivers/ata/ahci_imx\n"
		"+external       arch/powerpc/platforms/powernv/opal-prd\n"
	};

	const SupportedConf supp { supportedConf };

	assert(supp.supportState("non_existing") == SupportedConf::NonPresent);

	assert(supp.supportState("drivers/input/joystick/something.ko") ==
	       SupportedConf::Unsupported);
	assert(supp.supportState("drivers/mtd/spi-nor/spi-nor") == SupportedConf::Unspecified);
	assert(supp.supportState("drivers/mtd/something_else") == SupportedConf::Unsupported);
	assert(supp.supportState("drivers/usb/typec/altmodes/typec_displayport") ==
	       SupportedConf::Unsupported);
	assert(supp.supportState("fs/ocfs2/ocfs2_stackglue") == SupportedConf::KMPSupported);
	assert(supp.supportState("drivers/net/ethernet/8390/8390") == SupportedConf::BaseSupported);
	assert(supp.supportState("drivers/ata/ahci_imx") == SupportedConf::UnsupportedOptional);
	assert(supp.supportState("arch/powerpc/platforms/powernv/opal-prd") ==
	       SupportedConf::ExternallySupported);

	return 0;
}

