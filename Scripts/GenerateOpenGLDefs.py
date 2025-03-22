import re
import sys

def extract_function_names(file):
    function_names = []
    with open(file, 'r') as f:
        for line in f:
            match = re.match(r'PULSE_OPENGL_FUNCTION\((\w+),.*\)', line)
            if not match:
                match = re.match(r'PULSE_OPENGL_GL_GLES_FUNCTION\(.*?,.*?, (\w+),.*\)', line)
            if match:
                function_name = match.group(1)
                function_names.append(function_name)
    return function_names

def extract_prototypes(function_names, header_file):
    prototypes = {}
    with open(header_file, 'r') as f:
        for line in f:
            for function_name in function_names:
                if re.search(rf'\b{function_name}\b', line):
                    match = re.match(r'(.*?)\s*\b' + function_name + r'\b\s*\((.*?)\);', line)
                    if match:
                        return_type = match.group(1).replace('GL_APICALL ', '').replace(' GL_APIENTRY', '').replace('GL_APIENTRY', '')
                        arguments = match.group(2).split(', ')
                        argument_list = ["PulseDevice device"]
                        parameter_list = []
                        for argument in arguments:
                            parts = argument.split()
                            arg_name = parts[-1]
                            arg_type = ' '.join(parts[:-1])
                            if arg_name != 'void':
                                argument_list.append(f'{arg_type} {arg_name}')
                                parameter_list.append(f'{arg_name.split('*')[-1]}')
                        if return_type == 'void':
                            prototype = f'PULSE_OPENGL_WRAPPER({function_name}, ({", ".join(argument_list)}), ({", ".join(parameter_list)}), {"PFN" + function_name.upper() + "PROC"})'
                        else:
                            prototype = f'PULSE_OPENGL_WRAPPER_RET({return_type}, {function_name}, ({", ".join(argument_list)}), ({", ".join(parameter_list)}), {"PFN" + function_name.upper() + "PROC"})'
                        prototypes[function_name] = prototype
                        break
    return prototypes

def main():
    function_names = extract_function_names(sys.argv[1])
    prototypes = extract_prototypes(function_names, sys.argv[2])

    try:
        with open(sys.argv[3], "x") as f:
            f.write("""// Copyright (C) 2025 kanel
// This file is part of "Pulse"
// For conditions of distribution and use, see copyright notice in LICENSE

// This is a generated file

// No header guards

#ifndef PULSE_OPENGL_WRAPPER_RET
	#error "You must define PULSE_OPENGL_WRAPPER_RET before including this file"
#endif

#ifndef PULSE_OPENGL_WRAPPER
	#error "You must define PULSE_OPENGL_WRAPPER before including this file"
#endif\n
""")
            for function_name, prototype in prototypes.items():
                f.write(f'{prototype}\n')
    except FileExistsError:
        pass

if __name__ == '__main__':
    main()
