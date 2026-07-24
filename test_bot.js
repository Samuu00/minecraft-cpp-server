const mineflayer = require('mineflayer');

const bot = mineflayer.createBot({
  host: 'localhost',
  port: 25565,
  username: 'TestBot',
  version: '1.20.4'
});

bot.on('login', () => {
  console.log('Bot logged in!');
});

bot.on('spawn', async () => {
  console.log('Bot spawned! Digging a block...');
  
  // Find a block nearby to dig
  const target = bot.blockAt(bot.entity.position.offset(0, -1, 0));
  if (target && target.type !== 0) {
    try {
      await bot.dig(target);
      console.log('Finished digging!');
    } catch (err) {
      console.log('Dig error:', err);
    }
  } else {
    console.log('No block to dig');
  }
});

bot.on('error', err => console.log('Bot Error:', err));
bot.on('kicked', reason => console.log('Bot Kicked:', reason));
