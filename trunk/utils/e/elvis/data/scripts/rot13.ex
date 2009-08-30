"Defines rot13 alias to perform rot-13 encryption/decryption

alias rot13 {
 "Perform rot-13 encryption/decryption
 local i report=0 magic magicchar=^$.[* noignorecase
 for i (0..12)
 do {
  try eval !%s/(char(i + 'a'))/<TEMP>/g
  try eval !%s/(char(i + 'n'))/(char(i + 'a'))/g
  try eval !%s/<TEMP>/(char(i + 'n'))/g
  try eval !%s/(char(i + 'A'))/<temp>/g
  try eval !%s/(char(i + 'N'))/(char(i + 'A'))/g
  try eval !%s/<temp>/(char(i + 'N'))/g
 }
}
