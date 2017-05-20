static atomic_t scull_s_available = ATOMIC_INIT(1);
static int scull_s_open(struct inode *inode, struct file *filp)
{
 struct scull_dev *dev = &scull_s_device; /* device information */
 if (! atomic_dec_and_test (&scull_s_available)) {
 atomic_inc(&scull_s_available);
 return -EBUSY; /* already open */
 }
 /* then, everything else is copied from the bare scull device */
 if ( (filp->f_flags & O_ACCMODE) = = O_WRONLY)
 scull_trim(dev);
 filp->private_data = dev;
 return 0; /* success */
}

static int scull_s_release(struct inode *inode, struct file *filp)
{
 atomic_inc(&scull_s_available); /* release the device */
 return 0;
}
--------------------------------------------------------------------------------

spin_lock(&scull_u_lock);
 if (scull_u_count &&
 (scull_u_owner != current->uid) && /* allow user */
 (scull_u_owner != current->euid) && /* allow whoever did su */
 !capable(CAP_DAC_OVERRIDE)) { /* still allow root */
 spin_unlock(&scull_u_lock);
 return -EBUSY; /* -EPERM would confuse the user */
 }
 if (scull_u_count = = 0)
 scull_u_owner = current->uid; /* grab it */
 scull_u_count++;
 spin_unlock(&scull_u_lock);

 static int scull_u_release(struct inode *inode, struct file *filp)
{
 spin_lock(&scull_u_lock);
 scull_u_count--; /* nothing else */
 spin_unlock(&scull_u_lock);
return 0;
}

___________________________________________________________________________________

Blocking open as an Alternative to EBUSY

spin_lock(&scull_w_lock);
while (! scull_w_available( )) {
 spin_unlock(&scull_w_lock);
 if (filp->f_flags & O_NONBLOCK) return -EAGAIN;
 if (wait_event_interruptible (scull_w_wait, scull_w_available( )))
 return -ERESTARTSYS; /* tell the fs layer to handle it */
 spin_lock(&scull_w_lock);
}
if (scull_w_count = = 0)
 scull_w_owner = current->uid; /* grab it */
scull_w_count++;
spin_unlock(&scull_w_lock);

release
static int scull_w_release(struct inode *inode, struct file *filp)
{
 int temp;
 spin_lock(&scull_w_lock);
 scull_w_count--;
 temp = scull_w_count;
 spin_unlock(&scull_w_lock);
 if (temp = = 0)
  wake_up_interruptible_sync(&scull_w_wait); /* awake other uid's */
return 0;
}
