import os

from torch import nn, optim
import torch
from torch.utils.data import DataLoader
from dataset import *
from yolo_v3_net import *
from torch.utils.tensorboard import SummaryWriter

def loss_fun(output, target, c):
    output = output.permute(0, 2, 3, 1)
    output = output.reshape(output.size(0), output.size(1), output.size(2), 3, -1)

    mask_obj = target[..., 0] > 0
    mask_no_obj = target[..., 0] == 0

    loss_p_fun = nn.BCELoss()
    loss_p = loss_p_fun(torch.sigmoid(output[..., 0]), target[..., 0])

    loss_box_fun = nn.MSELoss()
    loss_box = loss_box_fun(output[mask_obj][..., 1:5], target[mask_obj][..., 1:5])

    loss_segment_fun = nn.CrossEntropyLoss()
    loss_segment = loss_segment_fun(output[mask_obj][..., 5:],
                                    torch.argmax(target[mask_obj][..., 5:], dim=1, keepdim=True).squeeze(dim=1))

    loss = c * loss_p + (1 - c) * 0.5 * loss_box + (1 - c) * 0.5 * loss_segment
    return loss

if __name__ == '__main__':
    summary_writer=SummaryWriter('logs')
    device = torch.device('cuda' if torch.cuda.is_available() else 'cpu')
    dataset = YoloDataSet()
    data_loader = DataLoader(dataset, batch_size=2, shuffle=True)

    weight_path = 'params/net.pt'
    net = Yolo_V3_Net().to(device)
    if os.path.exists(weight_path):
        net.load_state_dict(torch.load(weight_path))

    opt = optim.Adam(net.parameters())

    epoch=0
    index = 0
    while True:
        for target_13, target_26, target_52, img_data in data_loader:
            target_13, target_26, target_52, img_data = target_13.to(device), target_26.to(device), target_52.to(
                device), img_data.to(device)

            output_13, output_26, output_52 = net(img_data)

            loss_13 = loss_fun(output_13.float(), target_13.float(), 0.7)
            loss_26 = loss_fun(output_26.float(), target_26.float(), 0.7)
            loss_52 = loss_fun(output_52.float(), target_52.float(), 0.7)

            loss=loss_13+loss_26+loss_52

            opt.zero_grad()
            loss.backward()
            opt.step()

            print(f'loss{epoch}=={index}',loss.item())
            summary_writer.add_scalar('train_loss',loss,index)
            index+=1

        torch.save(net.state_dict(),'params/net.pt')
        print('模型保存成功')
        epoch+=1