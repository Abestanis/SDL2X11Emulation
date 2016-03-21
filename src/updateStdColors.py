from urllib2     import urlopen
from collections import OrderedDict

downloadUrl = 'https://cgit.freedesktop.org/xorg/app/rgb/plain/rgb.txt'
destFile = 'stdColors.h'
destFileTemplate = '''#ifndef _STD_COLORS_H_
#define _STD_COLORS_H_

// Values taken from http://cgit.freedesktop.org/xorg/app/rgb/tree/rgb.txt
// Names must be all lowercase, names which are missing spaces are taken out

typedef struct {
    char* name;
    unsigned long pixelValue;
} StdColorEntry;

static const StdColorEntry STANDARD_COLORS[] = {
{colors}
};

#define NUM_STANDARD_COLORS (sizeof(STANDARD_COLORS) / sizeof(STANDARD_COLORS[0]))

#endif /* _STD_COLORS_H_ */
'''

# Get the data
response = urlopen(downloadUrl)
data = response.read()

# Parse the data
colors = OrderedDict()
maxNameLen = 0;
for l in data.split('\n'):
    parts = l.split()
    if (len(parts) < 4):
        continue
    name = " ".join(parts[3:]).lower()
    if name in [x.replace(' ', '') for x in colors.keys() if x[0] == name[0]]:
        continue
    value = '0x' + hex(int(parts[0]))[2:].upper().zfill(2) +\
            hex(int(parts[1]))[2:].upper().zfill(2) + hex(int(parts[2]))[2:].upper().zfill(2) + 'FF'
    colors[name] = value
    maxNameLen = max(maxNameLen, len(name))

# Write colors to dest
colorText = ''
for colorName, colorValue in colors.items():
    colorText += '    {"' + colorName + '",' + (' ' * (maxNameLen - len(colorName) + 1)) + colorValue + '},\n'
with open(destFile, 'w') as dest:
    dest.write(destFileTemplate.replace('{colors}', colorText[:-1]))
