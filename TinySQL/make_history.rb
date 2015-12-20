dirs = []
Dir.glob('Answer/**').map do |path|
  file = File.basename(path)
  dirs[file.split('.')[0].to_i] = file
end

0.upto(0).each do |i|
  commit = dirs[i]
  p commit.encode('sjis')

  dif = Dir.entries("Answer\\#{commit}")
  p dif
  #puts `git add --all`
  #puts `git rm \"#{commit}\"`
  #puts `git commit \"#{commit}\"`
  #load 'make_doc.rb'
end