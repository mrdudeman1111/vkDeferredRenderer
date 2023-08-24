#pragma once

#include "Mesh.h"
#include <GLFW/glfw3.h>
#include <Pipeline.h>

namespace EkWidget
{
  class EkWidget
  {
    public:
      EkWidget();
      ~EkWidget();

      std::vector<Vertex2D> Vertices;

      glm::vec2 Position;
      uint32_t Width; // in pixels
      uint32_t Height; // in pixels

      virtual void OnClick();

      void Draw();
  };

  class EkButton : public EkWidget
  {
    public:
      EkButton();
      ~EkButton();

      void OnClick() override;
  };

  class EkUIPane
  {
    public:
      EkUIPane(Buffer inVBuff, Buffer inIBuff, VkDevice* inDevice);
      ~EkUIPane();

      std::vector<EkWidget> Widgets;

      Buffer vBuff;
      Buffer iBuff;

      void Draw(VkCommandBuffer* cmdBuffer, uint32_t Width, uint32_t Height);

      void ClickEvent(double MousePos[2]);

    private:
      VkDevice* pDevice;
  };
}

