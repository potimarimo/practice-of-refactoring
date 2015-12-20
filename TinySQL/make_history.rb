require 'fileutils'

dirs = []
Dir.glob('Answer/**').map do |path|
  file = File.basename(path)
  dirs[file.split('.')[0].to_i] = file
end

dirs.each do |dir|
  next unless dir
  p dir.encode('sjis')

  diff = Dir.entries("Answer/#{dir}")
  diff.each do |file|
  	case file
  	when 'ExecuteSQL.c'
      FileUtils.cp("Answer/#{dir}/#{file}", 'Solution/TinySQL')
      File.unlink('Solution/TinySQL/ExecuteSQL.cpp') if File.exist?('Solution/TinySQL/ExecuteSQL.cpp')
  	when 'ExecuteSQL.cpp'
  	  FileUtils.cp("Answer/#{dir}/#{file}", 'Solution/TinySQL')
      File.unlink('Solution/TinySQL/ExecuteSQL.c') if File.exist?('Solution/TinySQL/ExecuteSQL.c')
  	when 'TestTinySQL.cpp'
  	  FileUtils.cp("Answer/#{dir}/#{file}", 'Solution/TestTinySQL')
  	when 'TinySQL.vcxproj'
  	  FileUtils.cp("Answer/#{dir}/#{file}", 'Solution/TinySQL')
  	end
  end
  break
  #puts `git add --all`
  #puts `git rm \"#{commit}\"`
  #puts `git commit \"#{commit}\"`
  #load 'make_doc.rb'
end