"""Keep generated JVM source imports separate from executable code."""
import re

_LITERALS = re.compile(r'//[^\n]*|/\*.*?\*/|""".*?"""|"(?:\\.|[^"\\])*"|\'(?:\\.|[^\'\\])*\'', re.S)
_QUALIFIED = r'\borg\.basaltmc\.(?:server|proxy|shared)\.[A-Za-z_][\w]*(?:\.[A-Za-z_][\w]*)*'


def _code(text, pattern, replace, kotlin=False):
    out, at = [], 0
    for match in _LITERALS.finditer(text):
        out.append(re.sub(pattern, replace, text[at:match.start()]))
        literal = match[0]
        if kotlin and literal.startswith('"'):
            literal = re.sub(r'\$\{([^{}]+)\}', lambda m: '${' + re.sub(pattern, replace, m[1]) + '}', literal)
        out.append(literal)
        at = match.end()
    out.append(re.sub(pattern, replace, text[at:]))
    return ''.join(out)


def _type_path(path):
    parts = path.split('.')
    start = next((i for i, part in enumerate(parts) if part[0].isupper() and not part.isupper()), None)
    if start is None:
        return path, ''
    end = start + 1
    while end < len(parts) and parts[end][0].isupper() and not parts[end].isupper():
        end += 1
    return '.'.join(parts[:end]), '.'.join(parts[end:])


def java_imports(text):
    package = re.search(r'^package ([\w.]+);', text, re.M)
    package = package[1] if package else ''
    imports = set(re.findall(r'^import ([^;]+);', text, re.M))
    body = re.sub(r'^(?:package [^;]+;|import [^;]+;)\n?', '', text, flags=re.M).strip()
    bound = {path.split('.')[-1]: path for path in imports if not path.startswith('static ') and not path.endswith('.*')}
    local = set(re.findall(r'\b(?:class|interface|enum) (\w+)', body))

    def shorten(match):
        path, suffix = _type_path(match[0])
        name = path.split('.')[-1]
        if path.rsplit('.', 1)[0] != package:
            if name in local or (name in bound and bound[name] != path):
                return match[0]
            imports.add(path)
            bound[name] = path
        return name + ('.' + suffix if suffix else '')

    body = _code(body, _QUALIFIED + r'|\b(?:java\.(?:util|nio|lang)|org\.teavm)\.[A-Za-z_][\w]*(?:\.[A-Za-z_][\w]*)*', shorten)
    # Explicit inherited utility imports name the public API, never its implementation class.
    used = set(re.findall(r'\b\w+\b', _LITERALS.sub('', body)))
    imports = {path for path in imports if path.endswith('.*') or path.split('.')[-1] in used}
    normal = sorted(path for path in imports if not path.startswith('static '))
    static = sorted(path for path in imports if path.startswith('static '))
    blocks = ['package ' + package + ';'] if package else []
    for group in (normal, static):
        if group:
            blocks.append('\n'.join('import ' + path + ';' for path in group))
    blocks.append(body)
    return '\n\n'.join(blocks).rstrip() + '\n'


def kotlin_imports(text):
    package = re.search(r'^package ([\w.]+)', text, re.M)
    package = package[1] if package else ''
    imports = set(re.findall(r'^import ([^\n]+)', text, re.M))
    body = re.sub(r'^(?:package [\w.]+|import [^\n]+)\n?', '', text, flags=re.M).strip()
    annotations = []
    while body.startswith('@file:'):
        line, _, body = body.partition('\n')
        annotations.append(line)
        body = body.lstrip()
    bound = {path.rsplit(' as ', 1)[-1] if ' as ' in path else path.split('.')[-1]: path.split(' as ')[0] for path in imports if not path.endswith('.*')}
    local = set(re.findall(r'\b(?:fun|class|object|typealias) (\w+)', body))

    def shorten(match):
        path, suffix = _type_path(match[0])
        name = path.split('.')[-1]
        alias = None
        if path.endswith('.services.command'):
            alias = 'registerCommandWithCompletion'
        elif name in local or (name in bound and bound[name] != path):
            existing = next((name for name, target in bound.items() if target == path), None)
            alias = existing or path.split('.')[-2] + name[0].upper() + name[1:]
        name = alias or name
        if path.rsplit('.', 1)[0] != package or alias:
            imports.add(path + (' as ' + alias if alias else ''))
            bound[name] = path
        return name + ('.' + suffix if suffix else '')

    body = _code(body, _QUALIFIED + r'|\bkotlin\.(?:math|text|collections|wasm)\.[A-Za-z_][\w]*(?:\.[A-Za-z_][\w]*)*', shorten, kotlin=True)
    used = set(re.findall(r'\b\w+\b', body))
    imports = {path for path in imports if path.endswith('.*') or (path.rsplit(' as ', 1)[-1] if ' as ' in path else path.split('.')[-1]) in used}
    blocks = annotations[:]
    if package:
        blocks.append('package ' + package)
    if imports:
        blocks.append('\n'.join('import ' + path for path in sorted(imports)))
    blocks.append(body)
    return '\n\n'.join(blocks).rstrip() + '\n'


def csharp_imports(text):
    imports = set(re.findall(r'^using ([^;]+);', text, re.M))
    namespace = re.search(r'^namespace ([\w.]+);', text, re.M)
    namespace = namespace[1] if namespace else ''
    body = re.sub(r'^(?:using [^;]+;|namespace [\w.]+;)\n?', '', text, flags=re.M).strip()
    bound = {path.split(' = ')[0]: path.split(' = ')[1].removeprefix('global::') for path in imports if ' = ' in path}
    local = set(re.findall(r'\b(?:class|struct|interface|enum) (\w+)', body))

    def shorten(match):
        parts = match[0].removeprefix('global::').split('.')
        areas = {'Core', 'Values', 'Log', 'Text', 'Players', 'Data', 'Events', 'Tasks', 'Menus', 'Worlds', 'Items',
                 'Hud', 'Packets', 'Entities', 'Services', 'Commands', 'Config', 'Interaction', 'Admin', 'Network',
                 'Coins', 'Shop', 'Server'}
        at = 4 if len(parts) > 4 and parts[2] == 'Example' and parts[3] in areas else 3
        path, suffix = '.'.join(parts[:at + 1]), '.'.join(parts[at + 1:])
        name = path.split('.')[-1]
        if name in local or (name in bound and bound[name] != path):
            alias = path.split('.')[-2] + name
            imports.add(alias + ' = global::' + path)
            name = alias
        else:
            imports.add(path.rsplit('.', 1)[0])
        bound[name] = path
        return name + ('.' + suffix if suffix else '')

    body = _code(body, r'(?:global::)?\bBasalt\.(?:Server|Proxy|Shared)\.[A-Za-z_][\w]*(?:\.[A-Za-z_][\w]*)*', shorten)
    def standard(match):
        path = match[0]
        namespace, name = path.rsplit('.', 1)
        if name in local:
            return path
        imports.add(namespace)
        return name

    body = _code(body, r'\bSystem\.(?:Text\.Encoding|Array|BitConverter|Convert|Math)\b', standard)

    blocks = ['\n'.join('using ' + path + ';' for path in sorted(imports))] if imports else []
    if namespace:
        blocks.append('namespace ' + namespace + ';')
    blocks.append(body)
    return '\n\n'.join(blocks).rstrip() + '\n'
