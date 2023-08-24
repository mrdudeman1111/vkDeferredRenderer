#include "Pipeline.h"
#include <GLFW/glfw3.h>
#include <Widget.h>
#include <vulkan/vulkan_core.h>

namespace EkWidget
{
  EkUIPane::EkUIPane(Buffer inVBuff, Buffer inIBuff, VkDevice* inDevice) : vBuff(inVBuff), iBuff(inIBuff), pDevice(inDevice)
  {}

  EkUIPane::~EkUIPane()
  {}

  EkWidget::EkWidget()
  {}

  EkWidget::~EkWidget()
  {}

  EkButton::EkButton() : EkWidget()
  {}

  EkButton::~EkButton()
  {}

  void EkUIPane::ClickEvent(double MousePos[2])
  {
    for(uint32_t i = 0; i < Widgets.size(); i++)
    {
      uint32_t WidthOffset = Widgets[i].Width/2;

      if(MousePos[0] >= Widgets[i].Position.x - WidthOffset && MousePos[0] <= Widgets[i].Position.x + WidthOffset)
      {
        uint32_t HeightOffset = Widgets[i].Height/2;

        if(MousePos[1] >= Widgets[i].Position.y - HeightOffset && MousePos[1] <= Widgets[i].Position.y + HeightOffset)
        {
          //Widgets[i].OnClick();
        }
      }
    }
  }

  void EkUIPane::Draw(VkCommandBuffer* cmdBuffer, uint32_t Width, uint32_t Height)
  {
    std::vector<Vertex2D> Ret(0);

    Vertex2D* VertexMemory;
    vkMapMemory(*pDevice, *vBuff.Allocation.Memory, vBuff.Allocation.Offset, vBuff.Allocation.AllocSize, 0, (void**)&VertexMemory);

    uint32_t* IndexMemory;
    vkMapMemory(*pDevice, *iBuff.Allocation.Memory, iBuff.Allocation.Offset, iBuff.Allocation.AllocSize, 0, (void**)&IndexMemory);

    uint32_t vOffset = 0;
    uint32_t iOffset = 0;

    for(uint32_t i = 0; i < Widgets.size(); i++)
    {
      Widgets[i].Draw();

      memcpy(VertexMemory + vOffset, Widgets[i].Vertices.data(), Widgets[i].Vertices.size()*sizeof(Vertex2D));

      uint32_t Indices[6];
      Indices[0] = 0; // Bottom left
      Indices[1] = 1; // Top left
      Indices[2] = 2; // Top Right

      Indices[3] = 0; // Bottom left
      Indices[4] = 2; // Top right
      Indices[5] = 3; // Bottom right

      memcpy(IndexMemory + iOffset, Indices, sizeof(uint32_t) * 6);

      iOffset += 6;
      vOffset += Widgets[i].Vertices.size(); // should always be 4
    }

    vkUnmapMemory(*pDevice, *vBuff.Allocation.Memory);
    vkUnmapMemory(*pDevice, *iBuff.Allocation.Memory);

    // this vertex will only ever feed into one binding
    VkDeviceSize Offset = 0;
    vkCmdBindVertexBuffers(*cmdBuffer, 0, 1, &vBuff.Buffer, &Offset);

    vkCmdBindIndexBuffer(*cmdBuffer, iBuff.Buffer, Offset, VK_INDEX_TYPE_UINT32);

    uint32_t iDrawOffset = 0;
    uint32_t vDrawOffset = 0;

    for(uint32_t i = 0; i < Widgets.size(); i++)
    {
      vkCmdDrawIndexed(*cmdBuffer, 6, 0, iDrawOffset, vDrawOffset, 0);
    }
  }

  void EkWidget::OnClick()
  {

  }

  void EkWidget::Draw()
  {
    float DistanceX = Position.x/2.f;
    float DistanceY = Position.y/2.f;

    if(Vertices.size() != 4) Vertices.resize(4);

    Vertices[0].Data.Position.x = Position.x - DistanceX;
    Vertices[0].Data.Position.y = Position.y - DistanceY;

    Vertices[1].Data.Position.x = Position.x - DistanceX;
    Vertices[1].Data.Position.y = Position.y + DistanceY;

    Vertices[2].Data.Position.x = Position.x + DistanceX;
    Vertices[2].Data.Position.y = Position.y + DistanceY;

    Vertices[3].Data.Position.x = Position.x + DistanceX;
    Vertices[3].Data.Position.y = Position.y - DistanceY;
  }

  void EkButton::OnClick()
  {
    std::cout << "Button Click\n";
  }
}

