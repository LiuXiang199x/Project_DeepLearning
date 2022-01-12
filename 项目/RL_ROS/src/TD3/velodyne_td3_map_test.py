import os
import time
import numpy as np
import torch
import torch.nn as nn
import torch.nn.functional as F
from test_env_map import GazeboEnv
from replay_buffer3 import ReplayBuffer
from collections import deque
import subprocess

def evaluate(network, eval_episodes, epoch, sl_Process):
    avg_reward = 0.
    col = 0
    for _ in range(eval_episodes):
        count = 0
        state, cur_single_costmap, sl_Process = env.reset(sl_Process)
        state = state.reshape(-1, 1, 1)
        costmap = np.array([cur_single_costmap, cur_single_costmap, cur_single_costmap])
        done = False
        while not done and count < 501:
            action = network.get_action(np.array(state), costmap)
            a_in = [(action[0] + 1) / 2, action[1]]
            state, next_single_costmap, reward, done, _ = env.step(a_in)
            state = state.reshape(-1, 1, 1)
            next_costmap = costmap
            next_costmap[0:2,] = costmap[1:,]
            next_costmap[2,] = next_single_costmap
            costmap = next_costmap
            avg_reward += reward
            count += 1
            if reward < -90:
                col += 1
    avg_reward /= eval_episodes
    avg_col = col/eval_episodes
    print("..............................................")
    print("Average Reward over %i Evaluation Episodes, Epoch %i: %f, %f" % (eval_episodes, epoch, avg_reward, avg_col))
    print("..............................................")
    return avg_reward, sl_Process

class Flatten(nn.Module):
    def __init__(self):
        super().__init__()
    def forward(self, x):
        return x.view(x.shape[0], -1)


class Actor(nn.Module):

    def __init__(self, state_dim, action_dim, max_action):
        super(Actor, self).__init__()

        self.map_featuring = nn.Sequential(        # map_input: (bs*) 3*G*G, G=64
            nn.Conv2d(3, 64, 3, padding=1),        # 64*64*64
            nn.BatchNorm2d(64),
            nn.ReLU(),
            nn.MaxPool2d(kernel_size=2,stride=2),  # 64*32*32
            nn.Conv2d(64, 128, 3, padding=1),      # 128*32*32
            nn.BatchNorm2d(128),
            nn.ReLU(),
            nn.MaxPool2d(kernel_size=2,stride=2),  # 128*16*16
            nn.Conv2d(128, 256, 3, padding=1),     # 128*16*16
            nn.BatchNorm2d(256),
            nn.ReLU(),
            nn.MaxPool2d(kernel_size=2,stride=2),  # 128*8*8
            nn.Conv2d(256, 512, 8, padding=0),     # 512*1*1
        )

        self.all_input_handling = nn.Sequential(   # input: (bs*)*state_dim*1*1, state_dim = 512+35
            nn.Conv2d(state_dim, 512, 1, padding=0), # 512*1*1
            nn.BatchNorm2d(512),
            nn.ReLU(),
            nn.Conv2d(512, 512, 1, padding=0),       # 512*1*1
            nn.BatchNorm2d(512),
            nn.ReLU(),
            nn.Conv2d(512, action_dim, 1, padding=0), # action_dim*1*1
            nn.Tanh(),
        )

        self.max_action = max_action


    def forward(self, s, costmap):

        map_feature = self.map_featuring(costmap)
        all_features = torch.cat([map_feature, s], dim=1)        
        a = self.all_input_handling(all_features)
        return a


class Critic(nn.Module):

    def __init__(self, state_dim, action_dim):
        super(Critic, self).__init__()

        self.map_featuring_1 = nn.Sequential(        # input: (bs*) 3*G*G, G=64
            nn.Conv2d(3, 64, 3, padding=1),        # 64*64*64
            nn.BatchNorm2d(64),
            nn.ReLU(),
            nn.MaxPool2d(kernel_size=2,stride=2),  # 64*32*32
            nn.Conv2d(64, 128, 3, padding=1),      # 128*32*32
            nn.BatchNorm2d(128),
            nn.ReLU(),
            nn.MaxPool2d(kernel_size=2,stride=2),  # 128*16*16
            nn.Conv2d(128, 256, 3, padding=1),     # 128*16*16
            nn.BatchNorm2d(256),
            nn.ReLU(),
            nn.MaxPool2d(kernel_size=2,stride=2),  # 128*8*8
            nn.Conv2d(256, 512, 8, padding=0),     # 512*1*1
        )

        self.all_input_handling_1 = nn.Sequential(   # input: (bs*)*state_dim*1*1, state_dim = 512+XX
            nn.Conv2d(state_dim, 512, 1, padding=0), # 512*1*1
            nn.BatchNorm2d(512),
            nn.ReLU(),
            nn.Conv2d(512, 512, 1, padding=0),       # 512*1*1
            nn.BatchNorm2d(512),
            nn.ReLU(),
        )

        self.action_layer_1 = nn.Conv2d(action_dim, 128, 1, padding=0) # input (bs*)action_dim*1*1 , output (bs*)128*1*1
        self.all_layer_1 = nn.Sequential(
            nn.Conv2d(640, 1, 1, padding=0),
            Flatten(),         #(bs*)1
        )  # input (bs*)640*1*1 , output (bs*)1*1*1

        self.map_featuring_2 = nn.Sequential(        # map_input: (bs*) 3*G*G, G=64
            nn.Conv2d(3, 64, 3, padding=1),        # 64*64*64
            nn.BatchNorm2d(64),
            nn.ReLU(),
            nn.MaxPool2d(kernel_size=2,stride=2),  # 64*32*32
            nn.Conv2d(64, 128, 3, padding=1),      # 128*32*32
            nn.BatchNorm2d(128),
            nn.ReLU(),
            nn.MaxPool2d(kernel_size=2,stride=2),  # 128*16*16
            nn.Conv2d(128, 256, 3, padding=1),     # 128*16*16
            nn.BatchNorm2d(256),
            nn.ReLU(),
            nn.MaxPool2d(kernel_size=2,stride=2),  # 128*8*8
            nn.Conv2d(256, 512, 8, padding=0),     # 512*1*1

        )

        self.all_input_handling_2 = nn.Sequential(   # input: (bs*)*state_dim*1*1, state_dim = 512+XX
            nn.Conv2d(state_dim, 512, 1, padding=0), # 512*1*1
            nn.BatchNorm2d(512),
            nn.ReLU(),
            nn.Conv2d(512, 512, 1, padding=0),       # 512*1*1
            nn.BatchNorm2d(512),
            nn.ReLU(),
        )

        self.action_layer_2 = nn.Conv2d(action_dim, 128, 1, padding=0) # input (bs*)action_dim*1*1 , output (bs*)128*1*1

        self.all_layer_2 = nn.Sequential(
            nn.Conv2d(640, 1, 1, padding=0),
            Flatten(),                # (bs*)1*1*1
        )  # input (bs*)640*1*1 , output (bs*)1


    def forward(self, s, costmap, a):

        map_feature1 = self.map_featuring_1(costmap)
        all_features1 = torch.cat([map_feature1, s], dim=1)        
        all_features12 = self.all_input_handling_1(all_features1)
        action_features1 = self.action_layer_1(a)
        action_all_features1 = torch.cat([all_features12, action_features1], dim =1)
        q1 = self.all_layer_1(action_all_features1)

        map_feature2 = self.map_featuring_2(costmap)
        all_features2 = torch.cat([map_feature2, s], dim=1)        
        all_features22 = self.all_input_handling_2(all_features2)
        action_features2 = self.action_layer_2(a)
        action_all_features2 = torch.cat([all_features22, action_features2], dim =1)
        q2 = self.all_layer_2(action_all_features2)

        return q1, q2


# TD3 network
class TD3(object):

    def __init__(self, state_dim, action_dim, max_action):
        # Initialize the Actor network
        self.actor = Actor(state_dim, action_dim, max_action).to(device)
        self.actor_target = Actor(state_dim, action_dim, max_action).to(device)
        self.actor_target.load_state_dict(self.actor.state_dict())
        self.actor_optimizer = torch.optim.Adam(self.actor.parameters())

        # Initialize the Critic networks
        self.critic = Critic(state_dim, action_dim).to(device)
        self.critic_target = Critic(state_dim, action_dim).to(device)
        self.critic_target.load_state_dict(self.critic.state_dict())
        self.critic_optimizer = torch.optim.Adam(self.critic.parameters())

        self.max_action = max_action

    # Function to get the action from the actor
    def get_action(self, state, costmap):
        state = torch.Tensor(state.reshape(1, -1, 1, 1)).to(device)
        costmap = torch.Tensor(costmap.reshape(1,*costmap.shape[:])).to(device)
        return self.actor(state, costmap).cpu().data.numpy().flatten()

    # training cycle
    def train(self, replay_buffer, iterations, batch_size=100, discount=1, tau=0.005, policy_noise=0.2,  # discount=0.99
              noise_clip=0.5, policy_freq=2):
        for it in range(iterations):
            # sample a batch from the replay buffer
            batch_states, batch_maps, batch_actions, batch_rewards, batch_dones, batch_next_states, batch_next_maps = replay_buffer.sample_batch(
                batch_size)
            #print("batch_states", batch_states)
            #print(type(batch_states))
            #print(batch_states.shape)
            state = torch.Tensor(batch_states).to(device)
            costmap = torch.Tensor(batch_maps).to(device)
            next_state = torch.Tensor(batch_next_states).to(device)
            next_costmap = torch.Tensor(batch_next_maps).to(device)
            action = torch.Tensor(batch_actions).to(device)
            reward = torch.Tensor(batch_rewards).to(device)
            done = torch.Tensor(batch_dones).to(device)

            # Obtain the estimated action from the next state by using the actor-target
            next_action = self.actor_target(next_state, next_costmap)

            # Add noise to the action
            noise = torch.Tensor(batch_actions).data.normal_(0, policy_noise).to(device)
            noise = noise.clamp(-noise_clip, noise_clip)
            next_action = (next_action + noise).clamp(-self.max_action, self.max_action)

            # Calculate the Q values from the critic-target network for the next state-action pair
            target_Q1, target_Q2 = self.critic_target(next_state, next_costmap, next_action)

            # Select the minimal Q value from the 2 calculated values
            target_Q = torch.min(target_Q1, target_Q2)

            # Calculate the final Q value from the target network parameters by using Bellman equation
            target_Q = reward + ((1 - done) * discount * target_Q).detach()

            # Get the Q values of the basis networks with the current parameters
            current_Q1, current_Q2 = self.critic(state, costmap, action)

            # Calculate the loss between the current Q value and the target Q value
            loss = F.mse_loss(current_Q1, target_Q) + F.mse_loss(current_Q2, target_Q)

            # Perform the gradient descent
            self.critic_optimizer.zero_grad()
            loss.backward()
            self.critic_optimizer.step()

            if it % policy_freq == 0:
                # Maximize the the actor output value by performing gradient descent on negative Q values
                # (essentially perform gradient ascent)
                actor_grad, _ = self.critic(state, costmap, self.actor(state, costmap))
                actor_grad = -actor_grad.mean()
                self.actor_optimizer.zero_grad()
                actor_grad.backward()
                self.actor_optimizer.step()

                # Use soft update to update the actor-target network parameters by
                # infusing small amount of current parameters
                for param, target_param in zip(self.actor.parameters(), self.actor_target.parameters()):
                    target_param.data.copy_(tau * param.data + (1 - tau) * target_param.data)
                # Use soft update to update the critic-target network parameters by infusing
                # small amount of current parameters
                for param, target_param in zip(self.critic.parameters(), self.critic_target.parameters()):
                    target_param.data.copy_(tau * param.data + (1 - tau) * target_param.data)

    def save(self, filename, directory):
        torch.save(self.actor.state_dict(), '%s/%s_actor.pth' % (directory, filename))
        torch.save(self.critic.state_dict(), '%s/%s_critic.pth' % (directory, filename))

    def load(self, filename, directory):
        self.actor.load_state_dict(torch.load('%s/%s_actor.pth' % (directory, filename)))
        self.critic.load_state_dict(torch.load('%s/%s_critic.pth' % (directory, filename)))

    def train_mode(self):
        self.actor.train()
        self.actor_target.train()
        self.critic.train()
        self.critic_target.train()
    
    def eval_mode(self):
        self.actor.eval()
        self.actor_target.eval()
        self.critic.eval()
        self.critic_target.eval()


# Set the parameters for the implementation
device = torch.device("cuda" if torch.cuda.is_available() else "cpu")  # cuda or cpu
env_name = "HalfCheetahBulletEnv-v0"  # Name of the PyBullet environment. The network is updated for HalfCheetahBulletEnv-v0
seed = 0  # Random seed number
eval_freq = 5e3  # After how many steps to perform the evaluation
max_ep = 500
eval_ep = 10  # number of episodes for evaluation
max_timesteps = 5e6  # Maximum number of steps to perform
save_models = True  # Weather to save the model or not
expl_noise = 1  # Initial exploration noise starting value in range [expl_min ... 1]
expl_decay_steps = 500000  # Number of steps over which the initial exploration noise will decay over
expl_min = 0.1  # Exploration noise after the decay in range [0...expl_noise]
batch_size = 40  # Size of the mini-batch
discount = 0.99999  # Discount factor to calculate the discounted future reward (should be close to 1)
tau = 0.005  # Soft target update variable (should be close to 0)
policy_noise = 0.2  # Added noise for exploration
noise_clip = 0.5  # Maximum clamping values of the noise
policy_freq = 2  # Frequency of Actor network updates
buffer_size = 1e6  # Maximum size of the buffer
file_name = "TD3_velodyne"  # name of the file to store the policy
random_near_obstacle = True # To take random actions near obstacles or not

# Create the network storage folders
if not os.path.exists("./results"):
    os.makedirs("./results")
if save_models and not os.path.exists("./pytorch_models"):
    os.makedirs("./pytorch_models")

# Create the training environment
env = GazeboEnv('test_move_base4.launch', 1, 1, 1)
time.sleep(5)
# env.seed(seed)


port = '11311'
sl_Process = subprocess.Popen(["roslaunch", "-p", port, "/media/agent/eb0d0016-e15f-4a25-8c28-0ad31789f3cb/ROS/DRL-robot-navigation/TD3/assets/test_move_base5.launch"])
time.sleep(4)

torch.manual_seed(seed)
np.random.seed(seed)
state_dim = 24+512
action_dim = 2
max_action = 1

# Create the network
network = TD3(state_dim, action_dim, max_action)
# Create a replay buffer
replay_buffer = ReplayBuffer(buffer_size, seed)

# Create evaluation data store
evaluations = []

timestep = 0
timesteps_since_eval = 0
episode_num = 0
done = True
epoch = 1

count_rand_actions = 0
random_action = []


network.eval_mode()

# Begin the training loop
while timestep < max_timesteps:

    # On termination of episode
    if done:
        if timestep != 0:
            network.train_mode()
            network.train(replay_buffer, episode_timesteps, batch_size, discount, tau, policy_noise, noise_clip,
                          policy_freq)
            network.eval_mode()
        if timesteps_since_eval >= eval_freq:
            print("Validating")
            timesteps_since_eval %= eval_freq
            avg_reward1, sl_Process = evaluate(network, eval_ep, epoch, sl_Process)
            evaluations.append(avg_reward1)
            network.save(file_name, directory="./pytorch_models")
            np.save("./results/%s" % (file_name), evaluations)
            epoch += 1

        state, cur_single_costmap, sl_Process = env.reset(sl_Process)

        #print("state.shape",state.shape)
        #print(state)

        state = state.reshape(-1, 1, 1)
        costmap = np.array([cur_single_costmap, cur_single_costmap, cur_single_costmap])
        

        print("after reshaping state.shape",state.shape)
        print(type(state))

        done = False

        episode_reward = 0
        episode_timesteps = 0
        episode_num += 1

    if expl_noise > expl_min:
        expl_noise = expl_noise - ((1 - expl_min) / expl_decay_steps)

    action = network.get_action(np.array(state), costmap)
    action = (action + np.random.normal(0, expl_noise, size=action_dim)).clip(-max_action, max_action)

    # If the robot is facing an obstacle, randomly force it to take a consistent random action.
    # This is done to increase exploration in situations near obstacles.
    # Training can also be performed without it
    if random_near_obstacle:
        if np.random.uniform(0, 1) > 0.85 and min(state[4:-8,0,0]) < 0.6 and count_rand_actions < 1:
            count_rand_actions = np.random.randint(8, 15)
            random_action = np.random.uniform(-1, 1, 2)

        if count_rand_actions > 0:
            count_rand_actions -= 1
            action = random_action
            action[0] = -1

    # Update action to fall in range [0,1] for linear velocity and [-1,1] for angular velocity
    a_in = [(action[0] + 1) / 2, action[1]]
    next_state, next_single_costmap, reward, done, target = env.step(a_in)

    done_bool = 0 if episode_timesteps + 1 == max_ep else int(done)
    done = 1 if episode_timesteps + 1 == max_ep else int(done)
    episode_reward += reward

    next_state = next_state.reshape(-1, 1, 1)
    next_costmap = costmap
    next_costmap[0:2,] = costmap[1:,]
    next_costmap[2,] = next_single_costmap

    
    # Save the tuple in replay buffer
    action = action.reshape(-1, 1, 1)
    replay_buffer.add(state, costmap, action, reward, done_bool, next_state, next_costmap)

    # Update the counters
    state = next_state
    costmap = next_costmap
    episode_timesteps += 1
    timestep += 1
    timesteps_since_eval += 1

# After the training is done, evaluate the network and save it
avg_reward2, sl_Process = evaluate(network, eval_ep, 0, sl_Process)
evaluations.append(avg_reward2)
if save_models: network.save("%s" % (file_name), directory="./models")
np.save("./results/%s" % (file_name), evaluations)
sl_Process.terminate()

