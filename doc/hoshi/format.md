# Hoshi File Format Breakdown

Magic number: `7f 48 4f 53 48 49 7f` (`␡HOSHI␡`)

## Sections

- Magic Number
- Hoshi Version
- Constant Pool
- Global Variable Names
- Code
- Debugging Information (line counts)
- Notes (compiler, source language, etc etc)

## Byte Offsets

> Table design shoplifted from <https://en.wikipedia.org/wiki/Java_class_file>

**Notes:**

- Byte order is big-endian.
- Types are C type descriptors.

<table>
 <thead>
  <tr>
   <th>Byte Offset</th>
   <th>Size (bytes)</th>
   <th>Type and/or Value</th>
   <th>Description</th>
  </tr>
 </thead>
 <tbody>
  <!-- Magic number -->
  <tr>
   <td>0</td>
   <td rowspan=7>5</td>
   <td>uint8_t = 0x7f</td>
   <td rowspan=7>Magic Number</td>
  </tr>
  <tr>
   <td>1</td>
   <td>uint8_t = 0x48</td>
  </tr>
  <tr>
   <td>2</td>
   <td>uint8_t = 0x4f</td>
  </tr>
  <tr>
   <td>3</td>
   <td>uint8_t = 0x53</td>
  </tr>
  <tr>
   <td>4</td>
   <td>uint8_t = 0x48</td>
  </tr>
  <tr>
   <td>5</td>
   <td>uint8_t = 0x49</td>
  </tr>
  <tr>
   <td>6</td>
   <td>uint8_t = 0x7f</td>
  </tr>
  <!-- Major version -->
  <tr>
   <td>8</td>
   <td rowspan=2>2</td>
   <td rowspan=2>uint16_t</td>
   <td rowspan=2>Major Version</td>
  </tr>
  <tr>
   <td>9</td>
  </tr>
  <!-- Minor version -->
  <tr>
   <td>10</td>
   <td rowspan=2>2</td>
   <td rowspan=2>uint16_t</td>
   <td rowspan=2>Minor Version</td>
  </tr>
  <tr>
   <td>11</td>
  </tr>
  <!-- Constant pool size -->
  <tr>
   <td>12</td>
   <td rowspan=2>2</td>
   <td rowspan=2>uint16_t</td>
   <td rowspan=2>Number of entries in the constant pool</td>
  </tr>
  <tr>
   <td>13</td>
  </tr>
  <!-- Constant pool -->
  <tr>
   <td>14</td>
   <td>`cpsize` (variable)</td>
   <td>hoshi_Value[]</td>
   <td>List of values in the constant pool</td>
  </tr>
  <!-- Global variable name pool size -->
  <tr>
   <td>14+cpsize</td>
   <td rowspan=2>2</td>
   <td rowspan=2>uint16_t</td>
   <td rowspan=2>Number of entries in the global variable name pool</td>
  </tr>
  <tr>
   <td>15+cpsize</td>
  </tr>
  <!-- Global variable name pool -->
  <tr>
   <td>16+cpsize</td>
   <td>`gvnsize` (variable)</td>
   <td>struct { int length; char *chars; } []</td>
   <td>List of values in the global variable name pool</td>
  </tr>
  <!-- Instruction size -->
  <tr>
   <td>17+cpsize+gvnsize</td>
   <td rowspan=4>4</td>
   <td rowspan=4>uint32_t</td>
   <td rowspan=4>Number of instructions</td>
  </tr>
  <tr>
   <td>18+cpsize+gvnsize</td>
  </tr>
  <tr>
   <td>19+cpsize+gvnsize</td>
  </tr>
  <tr>
   <td>20+cpsize+gvnsize</td>
  </tr>
  <!-- Instructions -->
  <tr>
   <td>21+cpsize+gvnsize</td>
   <td>`isize` (variable)</td>
   <td>uint8_t[]</td>
   <td>List of instructions</td>
  </tr>
  <!-- Line marker count -->
  <tr>
   <td>21+cpsize+isize+gvnsize</td>
   <td rowspan=4>4</td>
   <td rowspan=4>uint32_t</td>
   <td rowspan=4>Amount of line markers</td>
  </tr>
  <tr>
   <td>22+cpsize+isize+gvnsize</td>
  </tr>
  <tr>
   <td>23+cpsize+isize+gvnsize</td>
  </tr>
  <tr>
   <td>24+cpsize+isize+gvnsize</td>
  </tr>
  <!-- Line markers -->
  <tr>
   <td>25+cpsize+isize+gvnsize</td>
   <td>`lsize` (variable)</td>
   <td>hoshi_LineStart[]</td>
   <td>List of line start markers</td>
  </tr>
  <!-- Notes -->
  <tr>
   <td>26+cpsize+isize+lsize+gvnsize</td>
   <td>`nsize` (variable)</td>
   <td>char[]</td>
   <td>Additional information added by the compiler, typically the compiler's name and the source language</td>
  </tr>
 </tbody>
</table>
