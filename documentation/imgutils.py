"""

'imgutils'

Utility macros to handle images, captions, and related things.

Author: Riccardo Maria BIANCHI <riccardo.maria.bianchi@cern.ch>

12 Jan 2021
"""

import math

def define_env(env):
    """
    Utility macros to handle images, captions, and related things.
    """

    # import the predefined macro
    try:
        fix_url = env.macros.fix_url # make relative urls point to root
    except AttributeError:
        # mkdocs-macros version < 5.10
        fix_url = env.variables.fix_url # make relative urls point to root
    
    # activate trace
    chatter = env.start_chatting("imgutils")


    @env.macro
    def imgutils_image(url:str, alt:str=''):
        "Add an image within <figure> tags. Optionally, you can set an alternate-text" 
        url2:str = url
        if urlFix : 
            url2 = fix_url(url)
            chatter("Fixing image's URL: %s --> %s" % (url, url2))
        return '<img src="%s", alt="%s">' % (url2, alt)

    @env.macro
    def imgutils_image_caption(url:str, alt:str='', width:int=400, cap:str='', urlFix:bool=True):
        "Add an image within <figure> tags. Optionally, you can set a caption, an alternate-text, and a width value." 
        url2:str = url
        if urlFix : 
            url2:str = fix_url(url)
            chatter("Fixing image's URL: %s --> %s" % (url, url2))
        return '<figure> <img src="%s", alt="%s", width="%s"> <figcaption>%s<figcaption> </figure>' % ( url2, alt, width, cap)


    @env.macro
    def imgutils_doc_env():
        """
        Document the environment, for development/debugging. 
        (Hint: Use it in your .md file within a code block, i.e. as: 
           ```
           {{ doc_env() | pprint }}
           ```
        """
        return {name:getattr(env, name) for name in dir(env) if not name.startswith('_')}


