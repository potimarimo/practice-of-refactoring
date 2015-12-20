require 'fileutils'

Dir.chdir('Solution/TinySQL/'){
  FileUtils.rm_r('.cccc') if Dir.exist?('.cccc')
  FileUtils.rm_r('html') if Dir.exist?('html')
  FileUtils.rm_r('xml') if Dir.exist?('xml')
  puts `dir /b /s | "%ProgramFiles(x86)%\\CCCC\\cccc.exe" -`
  puts `Doxygen`
}
































