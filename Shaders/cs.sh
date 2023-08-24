for vShader in *.vert
do
  glslc $vShader -o ${vShader%.*}.spv
done

for fShader in *.frag
do
  glslc $fShader -o ${fShader%.*}.spv
done
