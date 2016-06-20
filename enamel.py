import os
import json
import collections
import re
import sys
import array

try:
    from jinja2 import Environment
    from jinja2 import FileSystemLoader
except ImportError as e:
    if 'sdk-core' in sys.prefix :
        message = 'Jinja2 module is missing, you probably forgot to patch your current sdk\n'
        message += 'Fix the problem by executing the following command and relaunch your build:\n\n'
        message += 'pip install --target="%s/lib/python2.7/site-packages/" -r "%s/requirements.txt"\n' % (sys.prefix, os.path.dirname(os.path.abspath(__file__)))
        print message
        sys.exit(-1)
    else :
        raise e

def cvarname(name):
    """Convert a string to a valid c variable name (remove space,commas,slashes/...)."""
    return re.sub(r'([^\w\s]| )', '_', name)

def getid(item):
    """Return an identifier for the given config item, takes 'id' if it exists or 'messageKey'"""
    return item['id'] if 'id' in item else item['messageKey']

def maxdictsize(item):
    """Return the maximum size of the item in the dictionary"""
    size = 0
    if item['type'] == 'select' or item['type'] == 'radiogroup' :
        for option in item['options'] :
            size = max(size, len(str(option['value'])) + 1)
    return size

def getdefines(capabilities):
    """Generate the #define for the given capabilities"""
    if len(capabilities) == 0 :
        return "1"
    cap2defines = {
        "PLATFORM_APLITE"       : "defined(PBL_PLATFORM_APLITE)",
        "PLATFORM_BASALT"       : "defined(PBL_PLATFORM_BASALT)",
        "PLATFORM_CHALK"        : "defined(PBL_PLATFORM_CHALK)",
        "PLATFORM_DIORITE"      : "defined(PBL_PLATFORM_DIORITE)",
        "PLATFORM_EMERY"        : "defined(PBL_PLATFORM_EMERY)",
        "BW"                    : "defined(PBL_BW)",
        "COLOR"                 : "defined(PBL_COLOR)",
        "MICROPHONE"            : "defined(PBL_MICROPHONE)",
        "SMARTSTRAP"            : "defined(PBL_SMARTSTRAP)",
        "SMARTSTRAP_POWER"      : "defined(PBL_SMARTSTRAP_POWER)",
        "HEALTH"                : "defined(PBL_HEALTH)",
        "RECT"                  : "defined(PBL_RECT)",
        "ROUND"                 : "defined(PBL_ROUND)",
        "DISPLAY_144x168"       : "(defined(PBL_RECT) && !defined(PBL_PLATFORM_EMERY))",
        "DISPLAY_180x180_ROUND" : "(defined(PBL_ROUND) && defined(PBL_PLATFORM_CHALK))",
        "DISPLAY_200x228"       : "(defined(PBL_RECT) && defined(PBL_PLATFORM_EMERY))",
    }
    allcap2defines = {}
    for key, value in cap2defines.iteritems():
        allcap2defines[key]         = value
        allcap2defines['NOT_'+key]  = '!' + value
    return ' && '.join(allcap2defines[cap] for cap in capabilities) 

def getmessagekey(item):
    m = re.search(r"(.*)\[(\d+)\]", item['messageKey'])
    if m :
        return 'MESSAGE_KEY_' + m.group(1) + " + " + m.group(2)
    return 'MESSAGE_KEY_' + item['messageKey']

def settingscount(settings):
    count = 0
    for setting in settings :
        if setting['type'] == 'section':
            count = count + settingscount(setting['items'])
        elif 'messageKey' in setting :
            count = count + 1
    return count

def hashkey(item):
    messageKey = item['messageKey']
    if item['type'] == 'checkboxgroup' :
        messageKey = messageKey + '[' + str(len(item['options'])) + ']'
    return hash(messageKey) & 0xFFFFFFFF

def removeComments(string):
    """From http://stackoverflow.com/questions/2319019/using-regex-to-remove-comments-from-source-files"""
    string = re.sub(re.compile("/\*.*?\*/",re.DOTALL ) ,"" ,string) # remove all occurance streamed comments (/*COMMENT */) from string
    string = re.sub(re.compile("//.*?\n" ) ,"" ,string) # remove all occurance singleline comments (//COMMENT\n ) from string
    return string

def generate(configFile='src/js/config.json', outputDir='src/generated'):
    """Generates C helpers from a Clay configuration file"""
    # create output folder
    if not os.path.exists(outputDir):
        os.makedirs(outputDir)

    # create jinja environment
    env = Environment(loader = FileSystemLoader([os.path.join(os.path.dirname(__file__), 'templates')]), trim_blocks=True, lstrip_blocks=True)

    # add custom filters
    env.filters['cvarname'] = cvarname
    env.filters['getid']    = getid
    env.filters['maxdictsize']  = maxdictsize
    env.filters['getdefines'] = getdefines
    env.filters['getmessagekey'] = getmessagekey
    env.filters['hashkey'] = hashkey
    env.filters['settingscount'] = settingscount

    # load config file
    config_content=open(configFile)
    if configFile.endswith('.json') :
        # simply load the json file
        config_content=json.load(config_content)
    else :
        # Here we have a js file from Cloudpebble
        config_content=config_content.read()
        # Remove comments from js
        config_content=removeComments(config_content)
        # Export content of module.exports = [];
        config_content = re.findall('\s*module\.exports\s*=(.*);\s*',config_content,re.DOTALL)[0]
        config_content=json.loads(config_content)

    # render templates
    for template in ['enamel.h.jinja', 'enamel.c.jinja'] : 
    	extension = ".h" if template.endswith('h.jinja') else ".c" 
        f = open("%s/%s%s" % (outputDir, 'enamel', extension), 'w')
        f.write(env.get_template(template).render({'config' : config_content}))
        f.close()

def enamel(task):
    generate(configFile=task.inputs[0].abspath(), outputDir=task.generator.bld.bldnode.abspath())

import argparse
if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='Generates C helpers from a Clay configuration file')
    parser.add_argument('--config', action='store', default='src/js/config.json', help='Path to Clay configuration file') 
    parser.add_argument('--folder', action='store', default='.', help='Generation folder') 
    result = parser.parse_args()
    generate(configFile=result.config, outputDir=result.folder)
