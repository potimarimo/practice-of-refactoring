require 'fileutils'

dirs = []
Dir.glob('Answer/**').map do |path|
  file = File.basename(path)
  dirs[file.split('.')[0].to_i] = file
end

def delete(path)
  if File.exist?(path)
    File.unlink(path)
    puts `git rm \"#{path}\"`
  end
end

dirs.each.with_index do |dir, i|
  next unless dir
  diff = Dir.entries("Answer/#{dir}")
  diff.each do |file|
  	case file
  	when 'ExecuteSQL.c'
      FileUtils.cp("Answer/#{dir}/#{file}", 'Solution/TinySQL')
      delete 'Solution/TinySQL/ExecuteSQL.cpp'
  	when 'ExecuteSQL.cpp'
  	  FileUtils.cp("Answer/#{dir}/#{file}", 'Solution/TinySQL')
  	  delete 'Solution/TinySQL/ExecuteSQL.c'
  	when 'TestTinySQL.cpp'
  	  FileUtils.cp("Answer/#{dir}/#{file}", 'Solution/TestTinySQL')
  	when 'TinySQL.vcxproj'
  	  FileUtils.cp("Answer/#{dir}/#{file}", 'Solution/TinySQL')
  	end
  end  
  puts `git add --all`
  puts `git commit -m \"#{dir.encode('sjis')}\"`
end