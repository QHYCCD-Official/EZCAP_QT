import zipfile
from pathlib import Path
import xml.etree.ElementTree as ET

docx_path = Path(r'doc\\EZCAP_QT User Manual.docx')
with zipfile.ZipFile(docx_path) as zf:
    xml_data = zf.read('word/document.xml')

ns = {'w': 'http://schemas.openxmlformats.org/wordprocessingml/2006/main'}
root = ET.fromstring(xml_data)
paras = []
for p in root.findall('.//w:p', ns):
    texts = []
    for node in p.findall('.//w:t', ns):
        texts.append(node.text or '')
    if not texts:
        continue
    para = ''.join(texts).strip()
    if para:
        paras.append(para)
text = '\n'.join(paras)
Path('doc/manual_text.txt').write_text(text, encoding='utf-8')
