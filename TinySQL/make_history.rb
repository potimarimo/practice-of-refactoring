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
    puts 'delete ' + path
  end
end

def copy(src, dest)
  if(File.exist?(src))
    FileUtils.cp(src, dest)
    puts 'copy ' + dest + '/' + File.basename(src)
  end
end

dirs.each.with_index do |dir, i|
  next unless dir
  diff = Dir.entries("Answer/#{dir}")
  diff.each do |file|
  	case file
  	when 'ExecuteSQL.c'
      copy "Answer/#{dir}/#{file}", 'Answer/Solution/TinySQL'
      delete 'Answer/Solution/TinySQL/ExecuteSQL.cpp'
  	when 'ExecuteSQL.cpp'
  	  copy "Answer/#{dir}/#{file}", 'Answer/Solution/TinySQL'
  	  delete 'Answer/Solution/TinySQL/ExecuteSQL.c'
  	when 'TestTinySQL.cpp'
  	  copy "Answer/#{dir}/#{file}", 'Answer/Solution/TestTinySQL'
  	when 'TinySQL.vcxproj'
  	  copy "Answer/#{dir}/#{file}", 'Answer/Solution/TinySQL'
  	end
  end  
  puts `git add --all`
  puts `git commit -m \"#{dir.encode('sjis')}\"`
end